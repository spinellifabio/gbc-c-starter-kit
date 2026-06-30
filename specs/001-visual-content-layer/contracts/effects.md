# Contract: Effects API (`include/effects.h`)

Reusable effects over a single fixed `EffectState effects[MAX_EFFECTS]` array —
the ONLY WRAM the effects subsystem grows (FR-016). `effects_tick()` is called
once per V-Blank from the main loop.

| Function | Signature | Contract |
|----------|-----------|----------|
| `effects_init` | `void effects_init(void)` | Clears all slots to `EFFECT_NONE`. |
| `effects_tick` | `void effects_tick(void)` | Advances every active slot one frame. Detaches slots whose target was freed (FR-017). Cheap: `uint8_t` counters, O(MAX_EFFECTS). |
| `effect_blink` | `uint8_t effect_blink(uint8_t sprite_id, uint8_t period)` | Toggles target visibility every `period` frames (clamped ≥ 1, FR-018). Returns effect slot or `EFFECT_NONE` if full. Stops cleanly when freed (FR-012). |
| `effect_fade` | `uint8_t effect_fade(uint8_t direction, uint8_t frames)` | Ramps brightness to/from black; delegates to `utils.c` `fade_to_black`/`fade_from_black` (DMG-safe). `frames` clamped ≥ 1 (FR-013). |
| `effect_scroll` | `uint8_t effect_scroll(uint8_t bg_id, int8_t dx, int8_t dy, uint8_t period)` | Scrolls background per `examples/galaxy`, V-Blank timing (FR-014, no tearing). |
| `effect_anim` | `uint8_t effect_anim(uint8_t sprite_id, uint8_t first_tile, uint8_t frame_count, uint8_t period, uint8_t mode)` | Cycles frames at `period`; `mode` = `ANIM_LOOP`/`ANIM_ONCE` (FR-015). |
| `effect_stop` | `void effect_stop(uint8_t slot)` | Frees a slot; restores target to a clean state. |

**Invariants**: no heap, no per-effect growing allocation; `period`/`frames`
clamped to avoid divide-by-zero or non-terminating loops (FR-018); writing to a
freed target is prevented by the per-tick validity check (FR-017). **DMG**: fade
and any color-dependent effect degrade gracefully (grayscale / no-op).
