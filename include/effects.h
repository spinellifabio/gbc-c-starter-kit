#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdint.h>

/* Maximum simultaneous effects (only WRAM this module adds). */
#define MAX_EFFECTS     4u

/* Effect type constants */
#define EFFECT_NONE     0u
#define EFFECT_BLINK    1u
#define EFFECT_SCROLL   2u
#define EFFECT_ANIM     3u

/* effect_anim mode */
#define ANIM_LOOP       0u
#define ANIM_ONCE       1u

/* effect_fade direction */
#define FADE_OUT        0u
#define FADE_IN         1u

/* Returned when no free effect slot is available */
#define EFFECT_NO_SLOT  0xFFu

/*
 * Fixed-size effect state (data-model.md: ~44 B total for MAX_EFFECTS=4).
 * Only one of these arrays exists in WRAM — the entire effects WRAM budget.
 */
typedef struct {
    uint8_t type;        /* EFFECT_* type, EFFECT_NONE = free slot */
    uint8_t target;      /* OAM id (blink/anim) or BG layer id (scroll) */
    uint8_t period;      /* frames per step, always >= 1 */
    uint8_t counter;     /* frames elapsed since last step */
    uint8_t progress;    /* current animation frame or scroll step */
    uint8_t mode;        /* ANIM_LOOP/ONCE */
    uint8_t active;      /* 1 = slot running */
    uint8_t first_tile;  /* anim: starting tile */
    uint8_t frame_count; /* anim: total frames in strip */
    int8_t  scroll_dx;   /* scroll: x pixel delta per period */
    int8_t  scroll_dy;   /* scroll: y pixel delta per period */
} EffectState;

/* Init: zeros all effect slots. Call from game_system_init(). */
void effects_init(void);

/*
 * Advance all active effect slots by one frame.
 * Registered via add_VBL() — called once per V-Blank.
 * Does NOT handle fades (fade is blocking, call effect_fade() directly).
 */
void effects_vbl_tick(void);

/* Stop and free a specific effect slot. */
void effect_stop(uint8_t slot);

/* Detach all effects targeting the given sprite OAM id or BG layer id. */
void effects_detach_sprite(uint8_t oam_id);
void effects_detach_bg(uint8_t bg_id);

/*
 * Blink: toggle sprite visibility every `period` frames (>= 1).
 * Returns slot index, or EFFECT_NO_SLOT if the effects array is full.
 */
uint8_t effect_blink(uint8_t sprite_id, uint8_t period);

/*
 * Fade: blocking call that ramps brightness over `frames` frames.
 * Delegates to utils.c fade_to_black / fade_from_black (DMG-safe).
 * `frames` clamped to >= 1. Does NOT use the EffectState array.
 */
void effect_fade(uint8_t direction, uint8_t frames);

/*
 * Scroll: move a background by (dx, dy) every `period` frames.
 * Returns slot index, or EFFECT_NO_SLOT.
 */
uint8_t effect_scroll(uint8_t bg_id, int8_t dx, int8_t dy, uint8_t period);

/*
 * Animate: cycle frames first_tile..first_tile+frame_count-1 via sprite_set_tiles.
 * mode = ANIM_LOOP (repeat) or ANIM_ONCE (stop at last frame).
 * Returns slot index, or EFFECT_NO_SLOT.
 */
uint8_t effect_anim(uint8_t sprite_id, uint8_t first_tile, uint8_t frame_count,
                    uint8_t period, uint8_t mode);

#endif /* EFFECTS_H */
