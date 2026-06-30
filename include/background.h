#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <gb/gb.h>
#include <stdint.h>

/* Background layer IDs */
#define BG_LAYER_BG0 0u  /* game background content */
#define BG_LAYER_UI  1u  /* UI / text (managed by existing helpers; do not bg_load here) */

#define MAX_BG_LAYERS 2u

/*
 * BG tile-data partition (BG tile space is separate from sprite tile space).
 *
 * FONT_OFFSET = 128 (world_defs.h).  Tiles 128-255 are occupied by font and
 * the decorative tileset loaded in game_system_init().  Background content
 * tiles must live in 0-127 so they never overwrite the UI/font tiles — FR-006.
 */
#define BG_CONTENT_TILE_START 0u
#define BG_CONTENT_TILE_LIMIT 128u   /* exclusive upper bound; matches FONT_OFFSET */

/* Background scrolling modes */
typedef enum {
    BG_SCROLL_NONE = 0,
    BG_SCROLL_X,
    BG_SCROLL_Y,
    BG_SCROLL_XY
} bg_scroll_mode_t;

/* Background layer descriptor */
typedef struct {
    const uint8_t* data;      /* tile data (ROM) */
    const uint8_t* map;       /* tile map (ROM) */
    const uint8_t* attrmap;   /* CGB attribute map (ROM, may be NULL) */
    uint8_t width;            /* map width in tiles */
    uint8_t height;           /* map height in tiles */
    uint8_t scroll_x;
    uint8_t scroll_y;
    bg_scroll_mode_t scroll_mode;
    uint8_t palette;          /* BG palette slot (0-7) */
    uint8_t visible;          /* 0 = hidden, 1 = visible */
    uint8_t loaded;           /* 1 = tile data is in VRAM */
} background_t;

/*
 * bg_init — reset layer descriptors (no display or VRAM side-effects).
 * Call once from game_system_init().
 */
void bg_init(void);

/*
 * bg_load — load a background into the content tile range (0..127).
 *   layer    : BG_LAYER_BG0 (do not use BG_LAYER_UI).
 *   tiles    : ROM tile data; tile_count must be <= BG_CONTENT_TILE_LIMIT.
 *   tile_count : number of tiles in `tiles`.
 *   map      : ROM tile-index map (values are offsets into this bg's tiles).
 *   attrmap  : ROM CGB attribute map, or NULL.
 *   w, h     : map dimensions in tiles.
 *
 * Loads tile data at BG_CONTENT_TILE_START; writes via wait_vbl_done() — FR-008.
 * CGB attrmap loaded via VBK_REG bank 1 only when _cpu == CGB_TYPE — FR-007.
 */
void bg_load(uint8_t layer,
             const uint8_t* tiles, uint8_t tile_count,
             const uint8_t* map, const uint8_t* attrmap,
             uint8_t w, uint8_t h);

/*
 * bg_get — return the background_t* for the given layer, or NULL if invalid.
 * Used by the effects module to look up a layer by id.
 */
background_t* bg_get(uint8_t layer);

/* Scroll helpers (safe to call each frame; writes SCX/SCY in VBL window). */
void bg_set_scroll(background_t* bg, int8_t dx, int8_t dy);
void bg_set_scroll_mode(background_t* bg, bg_scroll_mode_t mode);

/* Visibility (toggles BG hardware show/hide for BG_LAYER_BG0). */
void bg_set_visible(background_t* bg, uint8_t visible);

/* Write a single tile to the map (deferred write, call inside VBL window). */
void bg_set_tile(background_t* bg, uint8_t x, uint8_t y, uint8_t tile);

/*
 * bg_set_palette — delegate to palette_alloc_bkg / palette_set_bkg.
 * On DMG, no-op for the color path.
 */
void bg_set_palette(background_t* bg, uint8_t palette);

/*
 * bg_clear — fill the BG map with tile 0 and reset the layer descriptor.
 * Does NOT touch font/UI tiles (>= BG_CONTENT_TILE_LIMIT) — FR-007.
 * Detaches any scroll effects targeting this layer.
 */
void bg_clear(uint8_t layer);

/* Update scroll registers (call once per frame from main loop if not in ISR). */
void bg_update(void);

#endif /* BACKGROUND_H */
