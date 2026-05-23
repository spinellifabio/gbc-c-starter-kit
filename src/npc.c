#include "npc.h"
#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "dialogue.h"
#include "lang.h"

static Npc npcs[MAX_NPCS];

/* Initialize one NPC slot. Returns 1 on success, 0 if index out of range. */
uint8_t npc_init(uint8_t idx, uint8_t x, uint8_t y,
                 uint8_t sprite_tile_base, uint8_t palette, uint8_t dialogue_id) {
    if (idx >= MAX_NPCS) return 0u;
    npcs[idx].x                = x;
    npcs[idx].y                = y;
    npcs[idx].active           = 1u;
    npcs[idx].anim_timer       = 0u;
    npcs[idx].sprite_tile      = sprite_tile_base;
    npcs[idx].sprite_tile_base = sprite_tile_base;
    npcs[idx].palette          = palette;
    npcs[idx].dialogue_id      = dialogue_id;
    return 1u;
}

/* Zero array then seed world NPCs. Call once from reset_gameplay(). */
void npc_init_all(void) {
    uint8_t i;
    for (i = 0u; i < MAX_NPCS; i++) {
        npcs[i].active = 0u;
    }
    /* World NPC definitions */
    npc_init(0u, 120u, 60u, 0u, 2u, (uint8_t)STR_NPC_GREETING);
}

/* 2-frame idle toggle every 30 frames (~0.5 s at 60 Hz) */
void npc_update(uint8_t idx) {
    if (idx >= MAX_NPCS || !npcs[idx].active) return;
    npcs[idx].anim_timer++;
    if (npcs[idx].anim_timer >= 30u) {
        npcs[idx].anim_timer = 0u;
        npcs[idx].sprite_tile =
            (npcs[idx].sprite_tile == npcs[idx].sprite_tile_base)
            ? npcs[idx].sprite_tile_base + 4u
            : npcs[idx].sprite_tile_base;
    }
}

/* Full 16x16 render. OAM base = NPC_OAM_BASE + (idx * 4). */
void npc_draw(uint8_t idx, uint16_t cam_x, uint16_t cam_y) {
    uint8_t oam;
    int16_t sx, sy;
    uint8_t dx, dy, t, p;

    if (idx >= MAX_NPCS) return;
    oam = (uint8_t)(NPC_OAM_BASE + (idx * 4u));

    if (!npcs[idx].active) {
        move_sprite(oam + 0u, 0u, 0u);
        move_sprite(oam + 1u, 0u, 0u);
        move_sprite(oam + 2u, 0u, 0u);
        move_sprite(oam + 3u, 0u, 0u);
        return;
    }

    sx = (int16_t)npcs[idx].x - (int16_t)cam_x;
    sy = (int16_t)npcs[idx].y - (int16_t)cam_y;

    if (sx < -8 || sx > (SCREENWIDTH + 8) ||
        sy < -16 || sy > (SCREENHEIGHT + 16)) {
        move_sprite(oam + 0u, 0u, 0u);
        move_sprite(oam + 1u, 0u, 0u);
        move_sprite(oam + 2u, 0u, 0u);
        move_sprite(oam + 3u, 0u, 0u);
        return;
    }

    dx = (uint8_t)sx;
    dy = (uint8_t)sy;
    t  = npcs[idx].sprite_tile;
    p  = npcs[idx].palette;

    /* 2x2 tile layout: top-left, top-right, bottom-left, bottom-right */
    set_sprite_tile(oam + 0u, t);
    set_sprite_tile(oam + 1u, t + 1u);
    set_sprite_tile(oam + 2u, t + 2u);
    set_sprite_tile(oam + 3u, t + 3u);

    move_sprite(oam + 0u, dx,      dy);
    move_sprite(oam + 1u, dx + 8u, dy);
    move_sprite(oam + 2u, dx,      dy + 8u);
    move_sprite(oam + 3u, dx + 8u, dy + 8u);

    set_sprite_prop(oam + 0u, p);
    set_sprite_prop(oam + 1u, p);
    set_sprite_prop(oam + 2u, p);
    set_sprite_prop(oam + 3u, p);
}

/* AABB collision: NPC hitbox 16x16 */
uint8_t npc_check_collision(uint8_t idx, uint8_t px, uint8_t py, uint8_t pw, uint8_t ph) {
    if (idx >= MAX_NPCS || !npcs[idx].active) return 0u;
    return (px + pw > npcs[idx].x && px < npcs[idx].x + 16u &&
            py + ph > npcs[idx].y && py < npcs[idx].y + 16u) ? 1u : 0u;
}

void npc_interact(uint8_t idx) {
    if (idx >= MAX_NPCS || !npcs[idx].active) return;
    dialogue_show_text(lang_str((LangStringId)npcs[idx].dialogue_id));
}

/* --- Bulk helpers --- */

void npc_update_all(void) {
    uint8_t i;
    for (i = 0u; i < MAX_NPCS; i++) npc_update(i);
}

void npc_draw_all(uint16_t cam_x, uint16_t cam_y) {
    uint8_t i;
    for (i = 0u; i < MAX_NPCS; i++) npc_draw(i, cam_x, cam_y);
}

/* Returns 1 if any active NPC's AABB overlaps the given rect */
uint8_t npc_solid_at(uint8_t px, uint8_t py, uint8_t pw, uint8_t ph) {
    uint8_t i;
    for (i = 0u; i < MAX_NPCS; i++) {
        if (npc_check_collision(i, px, py, pw, ph)) return 1u;
    }
    return 0u;
}
