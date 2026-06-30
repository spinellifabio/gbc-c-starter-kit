#include "effects.h"

#include <gb/gb.h>
#include <stdint.h>

#include "sprite.h"
#include "background.h"
#include "utils.h"

/* =========================================================
 * WRAM state  (44 B for MAX_EFFECTS=4 × EffectState — data-model.md)
 * This is the only WRAM the effects subsystem adds — FR-016.
 * ========================================================= */
static EffectState effects[MAX_EFFECTS];

/* ── Helpers ─────────────────────────────────────────────── */

static uint8_t alloc_slot(void) {
    uint8_t i;
    for (i = 0u; i < MAX_EFFECTS; i++) {
        if (!effects[i].active) return i;
    }
    return EFFECT_NO_SLOT;
}

/* =========================================================
 * Lifecycle
 * ========================================================= */

void effects_init(void) {
    uint8_t i;
    for (i = 0u; i < MAX_EFFECTS; i++) {
        effects[i].type   = EFFECT_NONE;
        effects[i].active = 0u;
    }
}

void effects_vbl_tick(void) {
    uint8_t i;
    for (i = 0u; i < MAX_EFFECTS; i++) {
        if (!effects[i].active) continue;

        effects[i].counter++;
        if (effects[i].counter < effects[i].period) continue;
        effects[i].counter = 0u;

        switch (effects[i].type) {

        case EFFECT_BLINK: {
            /* Toggle sprite visibility each period — FR-012. */
            Sprite* s = sprite_find(effects[i].target);
            if (!s) {
                /* Target freed — detach — FR-017. */
                effects[i].active = 0u;
            } else {
                uint8_t now_vis = (uint8_t)((effects[i].progress & 1u) ? 0u : 1u);
                sprite_set_visible(s, now_vis);
                effects[i].progress++;
            }
            break;
        }

        case EFFECT_SCROLL: {
            /* Advance scroll by (dx, dy) each period — FR-014. */
            background_t* bg = bg_get(effects[i].target);
            if (!bg) {
                effects[i].active = 0u;
            } else {
                bg_set_scroll(bg, effects[i].scroll_dx, effects[i].scroll_dy);
            }
            break;
        }

        case EFFECT_ANIM: {
            /* Cycle through animation frames — FR-015. */
            Sprite* s = sprite_find(effects[i].target);
            if (!s) {
                effects[i].active = 0u;
            } else {
                uint8_t frame = effects[i].progress;
                sprite_set_tiles(s, (uint8_t)(effects[i].first_tile + frame));
                frame++;
                if (frame >= effects[i].frame_count) {
                    if (effects[i].mode == ANIM_ONCE) {
                        effects[i].active = 0u;
                        frame = 0u;
                    } else {
                        frame = 0u; /* ANIM_LOOP */
                    }
                }
                effects[i].progress = frame;
            }
            break;
        }

        default:
            break;
        }
    }
}

void effect_stop(uint8_t slot) {
    if (slot >= MAX_EFFECTS) return;
    effects[slot].active = 0u;
    effects[slot].type   = EFFECT_NONE;
}

void effects_detach_sprite(uint8_t oam_id) {
    uint8_t i;
    for (i = 0u; i < MAX_EFFECTS; i++) {
        if (effects[i].active &&
            (effects[i].type == EFFECT_BLINK || effects[i].type == EFFECT_ANIM) &&
            effects[i].target == oam_id) {
            effects[i].active = 0u;
            effects[i].type   = EFFECT_NONE;
        }
    }
}

void effects_detach_bg(uint8_t bg_id) {
    uint8_t i;
    for (i = 0u; i < MAX_EFFECTS; i++) {
        if (effects[i].active &&
            effects[i].type == EFFECT_SCROLL &&
            effects[i].target == bg_id) {
            effects[i].active = 0u;
            effects[i].type   = EFFECT_NONE;
        }
    }
}

/* =========================================================
 * Individual effect starters
 * ========================================================= */

uint8_t effect_blink(uint8_t sprite_id, uint8_t period) {
    uint8_t slot = alloc_slot();
    if (slot == EFFECT_NO_SLOT) return EFFECT_NO_SLOT;

    effects[slot].type       = EFFECT_BLINK;
    effects[slot].target     = sprite_id;
    effects[slot].period     = (period == 0u) ? 1u : period; /* clamp >= 1 — FR-018 */
    effects[slot].counter    = 0u;
    effects[slot].progress   = 0u;
    effects[slot].mode       = 0u;
    effects[slot].active     = 1u;
    return slot;
}

void effect_fade(uint8_t direction, uint8_t frames) {
    /*
     * Blocking fade — delegates to utils.c (DMG-safe LUT).
     * Does NOT use the EffectState array — FR-013.
     * frames clamped >= 1 — FR-018.
     */
    if (frames == 0u) frames = 1u;
    if (direction == FADE_OUT) {
        fade_to_black(frames);
    } else {
        fade_from_black(frames);
    }
}

uint8_t effect_scroll(uint8_t bg_id, int8_t dx, int8_t dy, uint8_t period) {
    uint8_t slot;
    background_t* bg = bg_get(bg_id);
    if (!bg) return EFFECT_NO_SLOT;

    slot = alloc_slot();
    if (slot == EFFECT_NO_SLOT) return EFFECT_NO_SLOT;

    /* Ensure scroll mode allows the requested axes. */
    if (dx != 0 && dy != 0) {
        bg_set_scroll_mode(bg, BG_SCROLL_XY);
    } else if (dx != 0) {
        bg_set_scroll_mode(bg, BG_SCROLL_X);
    } else {
        bg_set_scroll_mode(bg, BG_SCROLL_Y);
    }

    effects[slot].type      = EFFECT_SCROLL;
    effects[slot].target    = bg_id;
    effects[slot].period    = (period == 0u) ? 1u : period;
    effects[slot].counter   = 0u;
    effects[slot].progress  = 0u;
    effects[slot].scroll_dx = dx;
    effects[slot].scroll_dy = dy;
    effects[slot].active    = 1u;
    return slot;
}

uint8_t effect_anim(uint8_t sprite_id, uint8_t first_tile, uint8_t frame_count,
                    uint8_t period, uint8_t mode) {
    uint8_t slot;
    if (frame_count == 0u) return EFFECT_NO_SLOT;

    slot = alloc_slot();
    if (slot == EFFECT_NO_SLOT) return EFFECT_NO_SLOT;

    effects[slot].type        = EFFECT_ANIM;
    effects[slot].target      = sprite_id;
    effects[slot].period      = (period == 0u) ? 1u : period; /* clamp >= 1 — FR-018 */
    effects[slot].counter     = 0u;
    effects[slot].progress    = 0u;
    effects[slot].mode        = mode;
    effects[slot].first_tile  = first_tile;
    effects[slot].frame_count = frame_count;
    effects[slot].active      = 1u;
    return slot;
}
