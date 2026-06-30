#include <gb/gb.h>
#include <gb/cgb.h>
#include <gbdk/font.h>

#include <stdint.h>

#include "player_idle.h"
#include "player_run.h"
#include "cat_idle.h"
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
    font_set(ibm_font);

    /* map_buffer on the stack — only needed during init, saves 12 bytes WRAM */
    uint8_t map_buffer[sizeof(tileset_map)];
    for (uint16_t i = 0u; i < sizeof(tileset_map); i++) {
        map_buffer[i] = (uint8_t)(tileset_map[i] + FONT_OFFSET);
    }

    set_bkg_data(FONT_OFFSET, tileset_TILE_COUNT, tileset_tiles);

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
     * 108-127: cat_idle          (20 tiles, placeholder)
     * 128-147: cutscene_player   (20 tiles, placeholder)
     * 148-167: cutscene_npc      (20 tiles, placeholder)
     */
    set_sprite_data(0u,                     player_idle_TILE_COUNT,    player_idle_tiles);
    set_sprite_data(player_idle_TILE_COUNT, player_run_TILE_COUNT,     player_run_tiles);
    set_sprite_data(TREASURE_TILE,          4u,                        treasure_tiles);
    set_sprite_data(HAZARD_TILE,            4u,                        hazard_tiles);
    set_sprite_data(CAT_IDLE_TILE,          cat_idle_TILE_COUNT,       cat_idle_tiles);
    /* Cutscene slots share player_idle tile data until final art is added */
    set_sprite_data(CUTSCENE_PLAYER_TILE, player_idle_TILE_COUNT, player_idle_tiles);
    set_sprite_data(CUTSCENE_NPC_TILE,    player_idle_TILE_COUNT, player_idle_tiles);

    if (_cpu == CGB_TYPE) {
        set_sprite_palette(0u, player_idle_PALETTE_COUNT, player_idle_palettes);
        set_sprite_palette(player_idle_PALETTE_COUNT, player_run_PALETTE_COUNT, player_run_palettes);
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
