#include "background.h"

#include <gb/gb.h>
#include <gb/cgb.h>
#include <string.h>

/* NOTE: background.c is currently unused by the game. It is excluded from
 * the CMake build (see CMakeLists.txt) to avoid wasting ~48 bytes WRAM.
 * Keep this file as a reference implementation for future use. */

#define MAX_BACKGROUNDS 3

static uint8_t is_cgb(void) {
    #ifdef CGB
    return (_cpu == CGB_TYPE) ? 1u : 0u;
    #else
    return 0u;
    #endif
}

static background_t backgrounds[MAX_BACKGROUNDS];
static uint8_t system_initialized = 0u;

// Initialize background system
void bg_init(void) {
    if (system_initialized) return;

    // Clear background array
    memset(backgrounds, 0, sizeof(backgrounds));

    // Initialize display registers
    DISPLAY_OFF;

    // Set up background and window display
    if (is_cgb()) {
        // CGB: Use BG and Window on both LCDC and Window layers
        BGP_REG = OBP0_REG = OBP1_REG = 0xE4;
        VBK_REG = 0;
    } else {
        // DMG: Simple monochrome palette
        BGP_REG = 0xE4;  // 11 10 01 00 (dark to light)
        OBP0_REG = 0xE4; // Sprite palette 0
        OBP1_REG = 0xE4; // Sprite palette 1 (unused)
    }

    // Clear VRAM
    fill_bkg_rect(0, 0, 32, 32, 0);

    // Enable display with background and sprites
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    system_initialized = 1u;
}

// Load a background from ROM
void bg_load(background_t* bg, uint8_t layer,
             const uint8_t* tiles, const uint8_t* map,
             const uint8_t* attrmap, uint8_t width, uint8_t height) {
    (void)bg; // Unused parameter
    if (layer >= MAX_BACKGROUNDS) return;

    // Initialize background structure
    background_t* target = &backgrounds[layer];
    target->data = (uint8_t*)tiles;
    target->map = (uint8_t*)map;
    target->attrmap = (uint8_t*)attrmap;
    target->width = width;
    target->height = height;
    target->x = 0;
    target->y = 0;
    target->scroll_x = 0;
    target->scroll_y = 0;
    target->scroll_mode = BG_SCROLL_NONE;
    target->palette = 0;
    target->visible = 1u;

    // Set VRAM bank and load tiles
    if (is_cgb()) {
        VBK_REG = 0; // Switch to tile data
    }

    // Load tile data to VRAM
    set_bkg_data(0, 255, tiles);

    // Load background map
    set_bkg_tiles(0, 0, width, height, map);

    // Load CGB attributes if available
    if (is_cgb() && attrmap) {
        VBK_REG = 1; // Switch to attribute data
        set_bkg_tiles(0, 0, width, height, attrmap);
        VBK_REG = 0; // Switch back to tile data
    }

    // Update display registers
    bg_update();
}

// Set background position
void bg_set_pos(background_t* bg, uint8_t x, uint8_t y) {
    if (!bg) return;
    bg->x = x;
    bg->y = y;
    bg_update();
}

// Set background scroll offset
void bg_set_scroll(background_t* bg, int8_t dx, int8_t dy) {
    if (!bg) return;

    // Update scroll position based on scroll mode
    switch (bg->scroll_mode) {
        case BG_SCROLL_X:
            bg->scroll_x = (bg->scroll_x + dx) & 0xFF;
            break;
        case BG_SCROLL_Y:
            bg->scroll_y = (bg->scroll_y + dy) & 0xFF;
            break;
        case BG_SCROLL_XY:
            bg->scroll_x = (bg->scroll_x + dx) & 0xFF;
            bg->scroll_y = (bg->scroll_y + dy) & 0xFF;
            break;
        default:
            // No scrolling
            break;
    }

    // Update hardware scroll registers if this is the active background
    if (bg == &backgrounds[BG_LAYER_BG0]) {
        SCX_REG = bg->scroll_x;
        SCY_REG = bg->scroll_y;
    }
}

// Set background visibility
void bg_set_visible(background_t* bg, uint8_t visible) {
    if (!bg) return;
    bg->visible = visible;

    // Update display registers
    if (bg == &backgrounds[BG_LAYER_BG0]) {
        if (visible) SHOW_BKG; else HIDE_BKG;
    }
}

// Set background scroll mode
void bg_set_scroll_mode(background_t* bg, bg_scroll_mode_t mode) {
    if (!bg) return;
    bg->scroll_mode = mode;
}

// Update background (call once per frame)
void bg_update(void) {
    // Update scroll registers for the main background
    SCX_REG = backgrounds[BG_LAYER_BG0].scroll_x;
    SCY_REG = backgrounds[BG_LAYER_BG0].scroll_y;

    // Additional updates for other layers if needed
    // ...
}

// Set background palette (DMG) or palette bank (CGB)
void bg_set_palette(background_t* bg, uint8_t palette) {
    if (!bg) return;
    bg->palette = palette;

    if (is_cgb()) {
        // On CGB, we need to update the attribute map
        // This is handled in bg_load() for now
    } else {
        // On DMG, we can set the background palette directly
        BGP_REG = palette;
    }
}

// Set CGB attributes for a tile
void bg_set_tile_attr(background_t* bg, uint8_t x, uint8_t y, uint8_t attr) {
    if (!bg || !is_cgb() || x >= bg->width || y >= bg->height) return;

    VBK_REG = 1; // Switch to attribute bank
    set_bkg_tiles(x, y, 1, 1, &attr);
    VBK_REG = 0; // Switch back to tile data
}

// Get CGB attributes for a tile
uint8_t bg_get_tile_attr(background_t* bg, uint8_t x, uint8_t y) {
    if (!bg || !is_cgb() || x >= bg->width || y >= bg->height) return 0;

    uint8_t attr;
    VBK_REG = 1; // Switch to attribute bank
    get_bkg_tiles(x, y, 1, 1, &attr);
    VBK_REG = 0; // Switch back to tile data
    return attr;
}

// Set a tile at the specified position
void bg_set_tile(background_t* bg, uint8_t x, uint8_t y, uint8_t tile) {
    if (!bg || x >= bg->width || y >= bg->height) return;

    set_bkg_tiles(x, y, 1, 1, &tile);
}

// Get a tile at the specified position
uint8_t bg_get_tile(background_t* bg, uint8_t x, uint8_t y) {
    if (!bg || x >= bg->width || y >= bg->height) return 0;

    uint8_t tile;
    get_bkg_tiles(x, y, 1, 1, &tile);
    return tile;
}

// Check if coordinates are within background bounds
uint8_t bg_in_bounds(const background_t* bg, uint8_t x, uint8_t y) {
    return (bg && x < bg->width && y < bg->height) ? 1u : 0u;
}


// Set CGB background palette colors
void bg_set_cgb_palette(uint8_t palette, uint16_t* colors) {
    if (!is_cgb() || !colors) return;

    // In GBDK-2020, we use cpdcc for CGB palette data
    // The colors array should contain 4 RGB555 colors (8 bytes total)
    // We'll set the palette directly using the GBC registers

    // Set the auto-incrementing palette index
    BCPS_REG = (palette << 3) | 0x80; // Auto-increment, start at color 0

    // Write the 4 colors (8 bytes) to the palette RAM
    for (uint8_t i = 0; i < 8; i++) {
        BCPD_REG = ((uint8_t *)colors)[i];
    }
}
