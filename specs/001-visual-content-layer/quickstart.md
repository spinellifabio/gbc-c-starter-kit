# Quickstart: Visual Content Layer — Validation Guide

Manual verification is the acceptance gate (constitution Principle II). No
automated tests. Use BGB (primary) and SameBoy; exercise both CGB and DMG modes.

## Prerequisites

- GBDK-2020 toolchain on PATH (`lcc`), CMake + Ninja.
- BGB and/or SameBoy installed (`cmake --build build --target run` launches BGB).

## Build & run

```bash
cmake --preset ninja-gbdk            # configure
cmake --build --preset build-rom     # build → rom/gbc-c-starter-kit.gbc
cmake --build build --target run     # launch in BGB
```

Build must be green and the ROM must boot before any scenario is checked.

## Scenario 1 — Sprite manager tile reuse & budget (US1 / SC-001..003)

1. Allocate two sprites with the **same** `graphic_id`; open BGB VRAM viewer.
2. **Expect**: tiles loaded once; both sprites render. (FR-002, SC-002)
3. Allocate sprites until the 88-tile budget / OAM<40 is exhausted.
4. **Expect**: further `sprite_alloc` returns `NULL`; no existing tile/slot
   overwritten. (FR-003/FR-004, SC-003)
5. Free one shared sprite; confirm tiles remain (ref_count > 0); free the last;
   confirm range released. (FR-005)

## Scenario 2 — Background ↔ UI/text coexistence (US2 / SC-004)

1. Load a background scene, then trigger the dialogue box typewriter.
2. **Expect**: text fully legible on top; background tiles never overwrite font
   tiles; dismissing dialogue leaves the background intact. (FR-006/FR-007)
3. Inspect VRAM viewer: BG content tiles sit in a range disjoint from
   `FONT_OFFSET`.

## Scenario 3 — Palettes CGB vs DMG (US3 / SC-005)

1. In BGB **CGB mode**: assign BG + OBJ palettes at runtime.
2. **Expect**: requested colors display; slot indices 0–7. (FR-009)
3. Request a 9th distinct BG palette → **expect** `PALETTE_NONE`, no corruption.
   (FR-011)
4. Switch BGB to **DMG mode**, rerun: content shows in grayscale, no crash, no
   garbled tiles. (FR-010, SC-005)

## Scenario 4 — Effects & WRAM bound (US4 / SC-006..008)

1. Blink a sprite → toggles at the configured cadence, stops cleanly. (FR-012)
2. Fade to/from black → smooth ramp on CGB, graceful on DMG. (FR-013)
3. Scroll a background continuously → smooth, **no tearing**. (FR-014, SC-008)
4. Animate a multi-frame sprite, `ANIM_LOOP` then `ANIM_ONCE`. (FR-015)
5. Free a sprite while an effect targets it → effect detaches, no write to freed
   slot. (FR-017)
6. Open the build memory map (`.map`/`.noi`): confirm total new WRAM ≈ the
   reported budget and the effects subsystem grows only `EffectState[MAX_EFFECTS]`.
   (FR-016, SC-007)

## Done criteria

- All four scenarios pass in BGB; Scenario 3 + fade re-checked in DMG mode.
- Build green; ROM boots; WRAM map within the data-model budget.
- Report which emulator(s), which scenarios, and the observed WRAM figure when
  claiming completion (Principle II).
