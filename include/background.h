#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <gb/gb.h>
#include <stdint.h>
/* No stdbool.h: GBDK/Z80 target uses uint8_t as boolean (0/1) */

// Background layer IDs
#define BG_LAYER_BG0 0
#define BG_LAYER_BG1 1
#define BG_LAYER_UI  2

// Background scrolling modes
typedef enum {
    BG_SCROLL_NONE,     // No scrolling
    BG_SCROLL_X,        // Horizontal only
    BG_SCROLL_Y,        // Vertical only
    BG_SCROLL_XY        // Both directions
} bg_scroll_mode_t;

// Background definition structure
typedef struct {
    uint8_t* data;          // Tile data
    uint8_t* map;           // Map data
    uint8_t* attrmap;       // Attribute map (CGB only)
    uint8_t width;          // Width in tiles
    uint8_t height;         // Height in tiles
    uint8_t x;              // X position
    uint8_t y;              // Y position
    uint8_t scroll_x;       // X scroll offset
    uint8_t scroll_y;       // Y scroll offset
    bg_scroll_mode_t scroll_mode;  // Scroll mode
    uint8_t palette;        // Background palette (DMG) / palette bank (CGB)
    uint8_t visible;        /* 0=hidden, 1=visible */
} background_t;

// Initialize background system
void bg_init(void);

// Load a background from ROM
void bg_load(background_t* bg, uint8_t layer, 
             const uint8_t* tiles, const uint8_t* map, 
             const uint8_t* attrmap, uint8_t width, uint8_t height);

// Set background position
void bg_set_pos(background_t* bg, uint8_t x, uint8_t y);

// Set background scroll offset
void bg_set_scroll(background_t* bg, int8_t dx, int8_t dy);

// Set background visibility
void bg_set_visible(background_t* bg, uint8_t visible);

// Set background scroll mode
void bg_set_scroll_mode(background_t* bg, bg_scroll_mode_t mode);

// Update background (call once per frame)
void bg_update(void);

// Set background palette (DMG) or palette bank (CGB)
void bg_set_palette(background_t* bg, uint8_t palette);

// Set CGB attributes for a tile
void bg_set_tile_attr(background_t* bg, uint8_t x, uint8_t y, uint8_t attr);

// Get CGB attributes for a tile
uint8_t bg_get_tile_attr(background_t* bg, uint8_t x, uint8_t y);

// Set a tile at the specified position
void bg_set_tile(background_t* bg, uint8_t x, uint8_t y, uint8_t tile);

// Get a tile at the specified position
uint8_t bg_get_tile(background_t* bg, uint8_t x, uint8_t y);

// Check if coordinates are within background bounds
uint8_t bg_in_bounds(const background_t* bg, uint8_t x, uint8_t y);

// Set CGB background palette colors
void bg_set_cgb_palette(uint8_t palette, uint16_t* colors);

#endif // BACKGROUND_H
