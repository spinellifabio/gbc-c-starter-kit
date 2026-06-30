#include "sprite.h"

#include <gb/gb.h>
#include <gb/metasprites.h>
#include <stdint.h>

#include "effects.h"

/* =========================================================
 * WRAM state  (data-model.md: ~352 B sprites + ~64 B tile table + 40 B oam bitmap)
 * ========================================================= */
static Sprite    sprites[MAX_SPRITES];
static TileAlloc tile_table[MAX_TILE_ALLOCS];
static uint8_t   oam_bitmap[40u]; /* 0 = free, 1 = in use by this manager */

/* ── Tile-allocation helpers ─────────────────────────────────────────── */

/* Returns the tile_table index for graphic_id, or MAX_TILE_ALLOCS if not found. */
static uint8_t tile_find(uint8_t graphic_id) {
    uint8_t i;
    for (i = 0u; i < MAX_TILE_ALLOCS; i++) {
        if (tile_table[i].ref_count > 0u && tile_table[i].graphic_id == graphic_id) {
            return i;
        }
    }
    return MAX_TILE_ALLOCS;
}

/* Returns the first free tile_table slot, or MAX_TILE_ALLOCS if full. */
static uint8_t tile_free_slot(void) {
    uint8_t i;
    for (i = 0u; i < MAX_TILE_ALLOCS; i++) {
        if (tile_table[i].ref_count == 0u) return i;
    }
    return MAX_TILE_ALLOCS;
}

/*
 * Find a contiguous gap of tile_count tiles at or above SPRITE_FIRST_FREE_TILE.
 * Returns the starting tile index, or 0 on failure (tile 0 is always reserved).
 *
 * Strategy: try SPRITE_FIRST_FREE_TILE, then the end of each active allocation.
 * O(MAX_TILE_ALLOCS^2) but MAX_TILE_ALLOCS=16 makes this negligible.
 */
static uint8_t tile_find_gap(uint8_t tile_count) {
    uint8_t candidates[MAX_TILE_ALLOCS + 1u];
    uint8_t nc = 0u;
    uint8_t i, ci, j;

    candidates[nc++] = SPRITE_FIRST_FREE_TILE;
    for (i = 0u; i < MAX_TILE_ALLOCS; i++) {
        if (tile_table[i].ref_count > 0u) {
            candidates[nc++] = (uint8_t)(tile_table[i].first_tile + tile_table[i].tile_count);
        }
    }

    for (ci = 0u; ci < nc; ci++) {
        uint8_t s = candidates[ci];
        if (s < SPRITE_FIRST_FREE_TILE) continue;
        /* Guard against wrap: s + tile_count must fit in 0..255 */
        if (tile_count > (uint8_t)(255u - s + 1u)) continue;

        uint8_t ok = 1u;
        for (j = 0u; j < MAX_TILE_ALLOCS; j++) {
            if (tile_table[j].ref_count == 0u) continue;
            uint8_t t0 = tile_table[j].first_tile;
            uint8_t t1 = (uint8_t)(t0 + tile_table[j].tile_count); /* exclusive end */
            /* Overlap: [s, s+count) intersects [t0, t1) */
            if (s < t1 && (uint8_t)(s + tile_count) > t0) {
                ok = 0u;
                break;
            }
        }
        if (ok) return s;
    }
    return 0u; /* not found */
}

/* ── OAM-allocation helpers ──────────────────────────────────────────── */

/* Find a free OAM slot (returns 0xFF if none). */
static uint8_t oam_alloc(void) {
    uint8_t i;
    for (i = 0u; i < 40u; i++) {
        if (!oam_bitmap[i]) {
            oam_bitmap[i] = 1u;
            return i;
        }
    }
    return 0xFFu;
}

static void oam_free(uint8_t id) {
    if (id < 40u) oam_bitmap[id] = 0u;
}

/* =========================================================
 * Public API
 * ========================================================= */

void sprite_init(void) {
    uint8_t i;
    for (i = 0u; i < MAX_SPRITES; i++) {
        sprites[i].id         = 0u;
        sprites[i].flags      = 0u;
        sprites[i].graphic_id = SPRITE_GRAPHIC_NONE;
        sprites[i].tile_id    = 0u;
        sprites[i].x         = 0u;
        sprites[i].y         = 0u;
        sprites[i].width     = 1u;
        sprites[i].height    = 1u;
        sprites[i].palette   = 0u;
        sprites[i].priority  = 0u;
        sprites[i].flip_x    = 0u;
        sprites[i].flip_y    = 0u;
    }
    for (i = 0u; i < MAX_TILE_ALLOCS; i++) {
        tile_table[i].ref_count = 0u;
    }
    for (i = 0u; i < 40u; i++) oam_bitmap[i] = 0u;

    /* Hide all OAM via the GBDK shadow buffer (hardware OAM is updated by VBL DMA). */
    hide_sprites_range(0u, 40u);
}

Sprite* sprite_alloc(uint8_t graphic_id, const uint8_t* tiles, uint8_t tile_count) {
    uint8_t ti, si, oam_id, first_tile;

    /* 1. Find or load the tile block. */
    ti = tile_find(graphic_id);
    if (ti == MAX_TILE_ALLOCS) {
        /* Not resident — find a gap and load. */
        ti = tile_free_slot();
        if (ti == MAX_TILE_ALLOCS) return NULL; /* tile table full */

        first_tile = tile_find_gap(tile_count);
        if (first_tile == 0u) return NULL; /* no tile gap in budget */

        set_sprite_data(first_tile, tile_count, tiles);
        tile_table[ti].graphic_id = graphic_id;
        tile_table[ti].first_tile = first_tile;
        tile_table[ti].tile_count = tile_count;
        tile_table[ti].ref_count  = 0u;
    }

    /* 2. Find a free OAM slot. */
    oam_id = oam_alloc();
    if (oam_id == 0xFFu) return NULL;

    /* 3. Find a free Sprite handle. */
    for (si = 0u; si < MAX_SPRITES; si++) {
        if (!(sprites[si].flags & SPRITE_FLAG_ACTIVE)) break;
    }
    if (si == MAX_SPRITES) {
        oam_free(oam_id);
        return NULL;
    }

    /* 4. Commit. */
    tile_table[ti].ref_count++;

    sprites[si].id         = oam_id;
    sprites[si].flags      = SPRITE_FLAG_ACTIVE | SPRITE_FLAG_VISIBLE;
    sprites[si].graphic_id = ti;
    sprites[si].tile_id    = tile_table[ti].first_tile;
    sprites[si].x         = 0u;
    sprites[si].y         = 0u;
    sprites[si].width     = 1u;
    sprites[si].height    = 1u;
    sprites[si].palette   = 0u;
    sprites[si].priority  = 0u;
    sprites[si].flip_x    = 0u;
    sprites[si].flip_y    = 0u;

    set_sprite_tile(oam_id, sprites[si].tile_id);
    move_sprite(oam_id, 0u, 0u);

    return &sprites[si];
}

void sprite_free(Sprite* sprite) {
    uint8_t ti;
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;

    /* Detach any active effects targeting this OAM slot — FR-017. */
    effects_detach_sprite(sprite->id);

    /* Hide and free OAM slot. */
    move_sprite(sprite->id, 0u, 0u);
    oam_free(sprite->id);

    /* Decrement tile ref count; release block when it reaches zero — FR-005. */
    ti = sprite->graphic_id;
    if (ti < MAX_TILE_ALLOCS && tile_table[ti].ref_count > 0u) {
        tile_table[ti].ref_count--;
    }

    sprite->flags = 0u;
}

void sprite_update(Sprite* sprite) {
    uint8_t flags;
    uint8_t x, y, tile;

    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;

    flags = (uint8_t)((sprite->flip_x ? S_FLIPX : 0u) |
                      (sprite->flip_y ? S_FLIPY : 0u) |
                      (sprite->priority ? S_PRIORITY : 0u));
    set_sprite_prop(sprite->id, (uint8_t)(flags | (sprite->palette & 0x07u)));

    if (sprite->flags & SPRITE_FLAG_VISIBLE) {
        move_sprite(sprite->id, sprite->x, sprite->y);
    } else {
        move_sprite(sprite->id, 0u, 0u);
    }

    /* Update sub-tiles for multi-tile sprites (2×2 max, id < 40). */
    if (sprite->width > 1u || sprite->height > 1u) {
        x    = sprite->x;
        y    = sprite->y;
        tile = sprite->tile_id;
        uint8_t hy, hx, sub_id;
        for (hy = 0u; hy < sprite->height; hy++) {
            for (hx = 0u; hx < sprite->width; hx++) {
                if (hx == 0u && hy == 0u) continue;
                sub_id = (uint8_t)(sprite->id + hy * 2u + hx);
                if (sub_id >= 40u) continue;
                set_sprite_prop(sub_id, (uint8_t)(flags | (sprite->palette & 0x07u)));
                set_sprite_tile(sub_id, (uint8_t)(tile + hy * 2u + hx));
                if (sprite->flags & SPRITE_FLAG_VISIBLE) {
                    move_sprite(sub_id, (uint8_t)(x + hx * 8u), (uint8_t)(y + hy * 8u));
                } else {
                    move_sprite(sub_id, 0u, 0u);
                }
            }
        }
    }
}

void sprite_update_all(void) {
    uint8_t i;
    for (i = 0u; i < MAX_SPRITES; i++) {
        if (sprites[i].flags & SPRITE_FLAG_ACTIVE) {
            sprite_update(&sprites[i]);
        }
    }
}

void sprite_clear_all(void) {
    /* Hide all 40 OAM entries via shadow buffer. */
    hide_sprites_range(0u, 40u);
}

void sprite_set_pos(Sprite* sprite, uint8_t x, uint8_t y) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    sprite->x = x;
    sprite->y = y;
    sprite_update(sprite);
}

void sprite_set_visible(Sprite* sprite, uint8_t visible) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    if (visible) {
        sprite->flags |= SPRITE_FLAG_VISIBLE;
        move_sprite(sprite->id, sprite->x, sprite->y);
    } else {
        sprite->flags &= (uint8_t)~SPRITE_FLAG_VISIBLE;
        move_sprite(sprite->id, 0u, 0u);
    }
}

void sprite_set_tiles(Sprite* sprite, uint8_t tile_id) {
    uint8_t hy, hx, sub_id;
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    sprite->tile_id = tile_id;
    set_sprite_tile(sprite->id, tile_id);
    if (sprite->width > 1u || sprite->height > 1u) {
        for (hy = 0u; hy < sprite->height; hy++) {
            for (hx = 0u; hx < sprite->width; hx++) {
                if (hx == 0u && hy == 0u) continue;
                sub_id = (uint8_t)(sprite->id + hy * 2u + hx);
                if (sub_id < 40u) {
                    set_sprite_tile(sub_id, (uint8_t)(tile_id + hy * 2u + hx));
                }
            }
        }
    }
}

void sprite_set_flip(Sprite* sprite, uint8_t flip_x, uint8_t flip_y) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    sprite->flip_x = flip_x ? 1u : 0u;
    sprite->flip_y = flip_y ? 1u : 0u;
    sprite_update(sprite);
}

void sprite_set_priority(Sprite* sprite, uint8_t priority) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    sprite->priority = priority ? 1u : 0u;
    sprite_update(sprite);
}

void sprite_set_palette(Sprite* sprite, uint8_t palette) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    sprite->palette = (uint8_t)(palette & 0x07u);
    sprite_update(sprite);
}

Sprite* sprite_find(uint8_t oam_id) {
    uint8_t i;
    for (i = 0u; i < MAX_SPRITES; i++) {
        if ((sprites[i].flags & SPRITE_FLAG_ACTIVE) && sprites[i].id == oam_id) {
            return &sprites[i];
        }
    }
    return NULL;
}
