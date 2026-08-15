#include <gb/gb.h>
#include <gb/cgb.h>
#include <gbdk/font.h>

#include <stdint.h>

#include "player_idle.h"
#include "player_run.h"
#include "cat_idle.h"
#include "cutscene_player.h"
#include "cutscene_npc.h"
#include "dialogue.h"
#include "audio.h"
#include "bgm.h"
#include "game_objects.h"
#include "game_settings.h"
#include "game_system.h"
#include "lang.h"
#include "save.h"
#include "tileset.h"
#include "world_defs.h"
#include "sprite.h"
#include "background.h"
#include "palette.h"
#include "effects.h"
#include "utils.h"
#include "logo.h"

/* Object OBJ palettes — slots 2/3 (data-model.md E4). Distinct from the
 * player's slots 0/1 so treasure/hazard no longer alias the player's colors. */
static const palette_color_t treasure_palette[4] = {
    RGB8(  0,  0,  0), RGB8(255,215,  0), RGB8(139, 69, 19), RGB8(255,255,255)
};
static const palette_color_t hazard_palette[4] = {
    RGB8(  0,  0,  0), RGB8(255,220,  0), RGB8(200, 40,  0), RGB8(255,255,255)
};

static void LCD_ISR(void) {
    /* Stub: add per-scanline work here (palette swaps, VRAM copy, wobble, etc.)
     * See examples/hblank_copy for a full usage example. */
}

void game_system_init(void) {
    cgb_compatibility();

    DISPLAY_OFF;

    /* Visual content layer — init data structures before any VRAM work. */
    sprite_init();
    bg_init();
    palette_init();
    effects_init();

    VBK_REG = 0u;

    font_init();
    font_t ibm_font = font_load(font_ibm);
    dialogue_set_window_base_tile(((pmfont_handle)ibm_font)->first_tile);
    text_set_font_base(((pmfont_handle)ibm_font)->first_tile);
    font_set(ibm_font);

    /* map_buffer on the stack — only needed during init, saves 12 bytes WRAM */
    uint8_t map_buffer[sizeof(tileset_map)];
    for (uint16_t i = 0u; i < sizeof(tileset_map); i++) {
        map_buffer[i] = (uint8_t)(tileset_map[i] + FONT_OFFSET);
    }

    set_bkg_data(FONT_OFFSET, tileset_TILE_COUNT, tileset_tiles);

    /* Title logo/banner — free BG tile range 96-103, disjoint from the font
     * (0-95) and the world tileset (128-131). Grayscale-safe pattern; no
     * palette dependency. */
    set_bkg_data(LOGO_TILE_BASE, LOGO_TILE_COUNT, logo_tiles);

    if (_cpu == CGB_TYPE) {
        set_bkg_palette(0u, tileset_PALETTE_COUNT, tileset_palettes);
        set_bkg_tiles(0u, 0u, tileset_MAP_ATTRIBUTES_WIDTH, tileset_MAP_ATTRIBUTES_HEIGHT, map_buffer);
        set_bkg_attributes(0u, 0u, tileset_MAP_ATTRIBUTES_WIDTH, tileset_MAP_ATTRIBUTES_HEIGHT, tileset_map_attributes);
    } else {
        set_bkg_tiles(0u, 0u, tileset_MAP_ATTRIBUTES_WIDTH, tileset_MAP_ATTRIBUTES_HEIGHT, map_buffer);
    }

    /* Sprite VRAM layout:
     *   0- 19: player_idle       (20 tiles)
     *  20- 99: player_run        (80 tiles)
     * 100-103: obj_treasure       (4 tiles)
     * 104-107: obj_hazard         (4 tiles)
     * 108-127: cat_idle          (20 tiles, recolored placeholder geometry)
     * 128-147: cutscene_player   (20 tiles, recolored placeholder geometry)
     * 148-167: cutscene_npc      (20 tiles, recolored placeholder geometry)
     */
    set_sprite_data(0u,                     player_idle_TILE_COUNT,    player_idle_tiles);
    set_sprite_data(player_idle_TILE_COUNT, player_run_TILE_COUNT,     player_run_tiles);
    set_sprite_data(TREASURE_TILE,          4u,                        treasure_tiles);
    set_sprite_data(HAZARD_TILE,            4u,                        hazard_tiles);
    set_sprite_data(CAT_IDLE_TILE,          cat_idle_TILE_COUNT,       cat_idle_tiles);
    set_sprite_data(CUTSCENE_PLAYER_TILE,   cutscene_player_TILE_COUNT, cutscene_player_tiles);
    set_sprite_data(CUTSCENE_NPC_TILE,      cutscene_npc_TILE_COUNT,    cutscene_npc_tiles);

    if (_cpu == CGB_TYPE) {
        set_sprite_palette(0u, player_idle_PALETTE_COUNT, player_idle_palettes);
        set_sprite_palette(player_idle_PALETTE_COUNT, player_run_PALETTE_COUNT, player_run_palettes);
        /* Slots 3-7: object/actor palettes. Slots 0-2 are taken by the
         * player (player_idle: slot 0; player_run: slots 1-2,
         * PALETTE_COUNT=2) — routed through the tracked allocator
         * (palette.c) so occupancy bookkeeping stays accurate. Previously
         * treasure/hazard used no palette of their own and aliased the
         * player's slots (palette bleed); cat/cutscene actors were never
         * given a palette at all. */
        palette_set_obj(3u, treasure_palette);
        palette_set_obj(4u, hazard_palette);
        palette_set_obj(5u, cat_idle_palettes);
        palette_set_obj(6u, cutscene_player_palettes);
        palette_set_obj(7u, cutscene_npc_palettes);
    }

    add_VBL(bgm_vbl_update);
    add_VBL(effects_vbl_tick);

    CRITICAL {
        LYC_REG  = 0u;
        STAT_REG |= STATF_LYC;
        add_LCD(LCD_ISR);
    }
    set_interrupts(IE_REG | VBL_IFLAG | LCD_IFLAG);

    SPRITES_8x16;
    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;

    lang_init(g_settings.language);
    save_init();
    audio_init();
    if (!g_settings.sound_on) audio_set_enabled(0u);
}
