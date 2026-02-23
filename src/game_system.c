#include <gb/gb.h>
#include <gb/cgb.h>
#include <gbdk/font.h>

#include <stdint.h>

#include "Alex_idle_16x16.h"
#include "Alex_run_16x16.h"
#include "dialogue.h"
#include "game_settings.h"
#include "game_system.h"
#include "lang.h"
#include "tileset.h"
#include "world_defs.h"

static uint8_t map_buffer[sizeof(tileset_map)];

void game_system_init(void) {
    cgb_compatibility();

    DISPLAY_OFF;

    VBK_REG = 0u;

    font_init();
    font_t ibm_font = font_load(font_ibm);
    dialogue_set_window_base_tile(((pmfont_handle)ibm_font)->first_tile);
    font_set(ibm_font);

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

    set_sprite_data(0u, Alex_idle_16x16_TILE_COUNT, Alex_idle_16x16_tiles);
    set_sprite_data(Alex_idle_16x16_TILE_COUNT, Alex_run_16x16_TILE_COUNT, Alex_run_16x16_tiles);

    if (_cpu == CGB_TYPE) {
        set_sprite_palette(0u, Alex_idle_16x16_PALETTE_COUNT, Alex_idle_16x16_palettes);
        set_sprite_palette(Alex_idle_16x16_PALETTE_COUNT, Alex_run_16x16_PALETTE_COUNT, Alex_run_16x16_palettes);
    }

    SPRITES_8x16;
    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;

    lang_init(g_settings.language);
}
