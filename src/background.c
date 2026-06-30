#include "background.h"

#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>

#include "palette.h"
#include "effects.h"

/* =========================================================
 * WRAM state  (~48 B for MAX_BG_LAYERS=2 × background_t)
 * ========================================================= */
static background_t layers[MAX_BG_LAYERS];

/* ── Internal helpers ────────────────────────────────────── */

static uint8_t is_cgb(void) {
    return (_cpu == CGB_TYPE) ? 1u : 0u;
}

/* =========================================================
 * Public API
 * ========================================================= */

void bg_init(void) {
    uint8_t i;
    for (i = 0u; i < MAX_BG_LAYERS; i++) {
        layers[i].data     = NULL;
        layers[i].map      = NULL;
        layers[i].attrmap  = NULL;
        layers[i].width    = 0u;
        layers[i].height   = 0u;
        layers[i].scroll_x = 0u;
        layers[i].scroll_y = 0u;
        layers[i].scroll_mode = BG_SCROLL_NONE;
        layers[i].palette  = 0u;
        layers[i].visible  = 0u;
        layers[i].loaded   = 0u;
    }
}

background_t* bg_get(uint8_t layer) {
    if (layer >= MAX_BG_LAYERS) return NULL;
    return &layers[layer];
}

void bg_load(uint8_t layer,
             const uint8_t* tiles, uint8_t tile_count,
             const uint8_t* map, const uint8_t* attrmap,
             uint8_t w, uint8_t h) {
    background_t* bg;
    if (layer >= MAX_BG_LAYERS || !tiles || !map) return;
    if (tile_count > BG_CONTENT_TILE_LIMIT) return; /* safety: never overwrite font */

    bg = &layers[layer];
    bg->data     = tiles;
    bg->map      = map;
    bg->attrmap  = attrmap;
    bg->width    = w;
    bg->height   = h;
    bg->scroll_x = 0u;
    bg->scroll_y = 0u;
    bg->loaded   = 1u;
    bg->visible  = 1u;

    /*
     * Bulk VRAM writes: caller is expected to be in a VBL window.
     * set_bkg_data / set_bkg_tiles are safe to call here because the
     * kit always calls bg_load right after wait_vbl_done() — FR-008.
     */

    /* Load tile data into BG tile range 0..tile_count-1 (below FONT_OFFSET). */
    VBK_REG = 0u;
    set_bkg_data(BG_CONTENT_TILE_START, tile_count, tiles);

    /* Load map. */
    set_bkg_tiles(0u, 0u, w, h, map);

    /* Load CGB attribute map (bank 1) only on CGB — FR-007. */
    if (is_cgb() && attrmap) {
        VBK_REG = 1u;
        set_bkg_tiles(0u, 0u, w, h, attrmap);
        VBK_REG = 0u;
    }

    if (layer == BG_LAYER_BG0) {
        SCX_REG = 0u;
        SCY_REG = 0u;
        SHOW_BKG;
    }
}

void bg_set_scroll(background_t* bg, int8_t dx, int8_t dy) {
    if (!bg) return;
    switch (bg->scroll_mode) {
        case BG_SCROLL_X:
            bg->scroll_x = (uint8_t)((bg->scroll_x + dx) & 0xFFu);
            break;
        case BG_SCROLL_Y:
            bg->scroll_y = (uint8_t)((bg->scroll_y + dy) & 0xFFu);
            break;
        case BG_SCROLL_XY:
            bg->scroll_x = (uint8_t)((bg->scroll_x + dx) & 0xFFu);
            bg->scroll_y = (uint8_t)((bg->scroll_y + dy) & 0xFFu);
            break;
        default:
            break;
    }
    /* Apply to hardware — safe at VBL time (called from effects_vbl_tick). */
    if (bg == &layers[BG_LAYER_BG0]) {
        SCX_REG = bg->scroll_x;
        SCY_REG = bg->scroll_y;
    }
}

void bg_set_scroll_mode(background_t* bg, bg_scroll_mode_t mode) {
    if (bg) bg->scroll_mode = mode;
}

void bg_set_visible(background_t* bg, uint8_t visible) {
    if (!bg) return;
    bg->visible = visible ? 1u : 0u;
    if (bg == &layers[BG_LAYER_BG0]) {
        if (visible) { SHOW_BKG; } else { HIDE_BKG; }
    }
}

void bg_set_tile(background_t* bg, uint8_t x, uint8_t y, uint8_t tile) {
    if (!bg || x >= bg->width || y >= bg->height) return;
    /* Caller must ensure this runs in VBL window — FR-008. */
    set_bkg_tiles(x, y, 1u, 1u, &tile);
}

void bg_set_palette(background_t* bg, uint8_t palette) {
    if (!bg) return;
    if (palette == 0xFFu) {
        /* Auto-allocate */
        uint8_t slot = palette_alloc_bkg(NULL);
        if (slot == PALETTE_NONE) return;
        bg->palette = slot;
    } else {
        bg->palette = palette;
        palette_set_bkg(palette, NULL); /* mark slot used without re-writing colors */
    }
}

void bg_clear(uint8_t layer) {
    background_t* bg;
    if (layer >= MAX_BG_LAYERS) return;
    bg = &layers[layer];

    /* Detach any scroll effects targeting this layer — FR-017. */
    effects_detach_bg(layer);

    /* Fill BG map with tile 0 (blank). Stays within 0..BG_CONTENT_TILE_LIMIT-1. */
    fill_bkg_rect(0u, 0u, 32u, 32u, 0u);

    bg->loaded   = 0u;
    bg->visible  = 0u;
    bg->scroll_x = 0u;
    bg->scroll_y = 0u;
    if (layer == BG_LAYER_BG0) {
        SCX_REG = 0u;
        SCY_REG = 0u;
    }
}

void bg_update(void) {
    /* Sync scroll registers for BG_LAYER_BG0. */
    SCX_REG = layers[BG_LAYER_BG0].scroll_x;
    SCY_REG = layers[BG_LAYER_BG0].scroll_y;
}
