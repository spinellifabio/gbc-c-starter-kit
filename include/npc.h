#ifndef NPC_H
#define NPC_H

#include <gb/gb.h>
#include <stdint.h>

/* OAM slots 24-39: 4 slots per NPC (2x2 sprite) */
#define NPC_OAM_BASE  24u
#define MAX_NPCS       4u

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t active;
    uint8_t anim_timer;
    uint8_t sprite_tile;        /* current base tile (toggled by animation) */
    uint8_t sprite_tile_base;   /* frame A tile (animation origin) */
    uint8_t palette;            /* CGB palette index (0-based) */
    uint8_t dialogue_id;        /* cast to LangStringId on use */
} Npc;

/* Initialize one NPC slot. Returns 1 on success, 0 if index out of range. */
uint8_t npc_init(uint8_t idx, uint8_t x, uint8_t y,
                 uint8_t sprite_tile_base, uint8_t palette, uint8_t dialogue_id);

/* Zero array + seed world NPCs. Call from reset_gameplay(). */
void npc_init_all(void);

/* Per-NPC functions */
void    npc_update(uint8_t idx);
void    npc_draw(uint8_t idx, uint16_t cam_x, uint16_t cam_y);
uint8_t npc_check_collision(uint8_t idx, uint8_t px, uint8_t py, uint8_t pw, uint8_t ph);
void    npc_interact(uint8_t idx);

/* Bulk helpers — iterate all active NPCs */
void npc_update_all(void);
void npc_draw_all(uint16_t cam_x, uint16_t cam_y);

/* Returns 1 if any active NPC blocks the given AABB (solid collision) */
uint8_t npc_solid_at(uint8_t px, uint8_t py, uint8_t pw, uint8_t ph);

#endif /* NPC_H */
