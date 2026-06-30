#ifndef SPRITE_H
#define SPRITE_H

#include <gb/gb.h>
#include <stdint.h>

/* ── OAM constants ─────────────────────────────────────────────────────── */
#define MAX_SPRITES         16u   /* logical sprites the manager can track */

/* Status flags (internal) */
#define SPRITE_FLAG_ACTIVE  0x01u
#define SPRITE_FLAG_VISIBLE 0x02u

/* Sentinel graphic_id meaning "no tile allocation" */
#define SPRITE_GRAPHIC_NONE 0xFFu

/* ── VRAM tile budget ──────────────────────────────────────────────────── */
/*
 * Sprite VRAM tiles 0-167 are reserved by game_system_init() for the
 * existing game assets.  The sprite manager allocates new graphics only
 * from tile 168 upward (88 tiles free of 256).
 *
 *   0- 19: player_idle (20)     100-103: obj_treasure (4)
 *  20- 99: player_run  (80)     104-107: obj_hazard   (4)
 * 108-127: cat_idle (20)        128-167: cutscene placeholders (40)
 */
#define SPRITE_FIRST_FREE_TILE  168u

/* ── Tile allocation table ─────────────────────────────────────────────── */
#define MAX_TILE_ALLOCS     16u   /* distinct graphics the manager can track */

typedef struct {
    uint8_t graphic_id;   /* caller-assigned graphic identity key */
    uint8_t first_tile;   /* first VRAM sprite tile index */
    uint8_t tile_count;   /* number of tiles in this block */
    uint8_t ref_count;    /* sprites currently referencing this block */
} TileAlloc;

/* ── Sprite handle ─────────────────────────────────────────────────────── */
typedef struct {
    uint8_t id;           /* primary OAM index */
    uint8_t flags;        /* SPRITE_FLAG_* */
    uint8_t graphic_id;   /* index into TileAlloc table (SPRITE_GRAPHIC_NONE if none) */
    uint8_t tile_id;      /* first tile (cached from alloc record) */
    uint8_t x;
    uint8_t y;
    uint8_t width;        /* tiles wide (1 or 2) */
    uint8_t height;       /* tiles tall (1 or 2) */
    uint8_t palette;      /* CGB OBJ palette slot (0-7) */
    uint8_t priority;     /* 0 = above BG, 1 = behind BG */
    uint8_t flip_x;
    uint8_t flip_y;
} Sprite;

/* ── Lifecycle ─────────────────────────────────────────────────────────── */

/*
 * sprite_init — zero tables, hide all OAM via shadow buffer.
 * Call once from game_system_init() after DISPLAY_OFF.
 */
void sprite_init(void);

/*
 * sprite_alloc — allocate a sprite handle.
 *   graphic_id : caller-defined key for this graphic (used for tile reuse).
 *   tiles      : pointer to tile data in ROM (const; loaded via set_sprite_data).
 *   tile_count : number of tiles to load (must fit within 88-tile budget).
 *
 * Returns a handle on success.
 * Returns NULL if no free OAM slot or no tile gap fits the budget.
 *
 * If graphic_id is already resident, reuses its tiles (no new VRAM write)
 * and increments the ref count — FR-002.
 */
Sprite* sprite_alloc(uint8_t graphic_id, const uint8_t* tiles, uint8_t tile_count);

/*
 * sprite_free — release OAM slot + decrement tile ref count.
 * Tile block is released when ref count reaches zero — FR-005.
 * Detaches any active effects targeting this sprite — FR-017.
 */
void sprite_free(Sprite* sprite);

/* ── Per-frame sync ────────────────────────────────────────────────────── */
void sprite_update(Sprite* sprite);
void sprite_update_all(void);
void sprite_clear_all(void);

/* ── Property setters ──────────────────────────────────────────────────── */
void sprite_set_pos(Sprite* sprite, uint8_t x, uint8_t y);
void sprite_set_visible(Sprite* sprite, uint8_t visible);
void sprite_set_tiles(Sprite* sprite, uint8_t tile_id);
void sprite_set_flip(Sprite* sprite, uint8_t flip_x, uint8_t flip_y);
void sprite_set_priority(Sprite* sprite, uint8_t priority);
void sprite_set_palette(Sprite* sprite, uint8_t palette);

/* ── Lookup ────────────────────────────────────────────────────────────── */

/*
 * sprite_find — returns the active Sprite with the given primary OAM index,
 * or NULL if no active sprite has that id.
 * Used by the effects module to validate targets before operating on them.
 */
Sprite* sprite_find(uint8_t oam_id);

#endif /* SPRITE_H */
