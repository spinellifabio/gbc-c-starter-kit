---

description: "Task list for Visual Content Layer implementation"
---

# Tasks: Visual Content Layer

**Input**: Design documents from `specs/001-visual-content-layer/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: No automated test suite (constitution Principle II — manual BGB/SameBoy
verification is the gate). No test tasks generated; verification tasks live in
the Polish phase and follow `quickstart.md`.

**Organization**: Tasks grouped by user story (US1 sprite manager, US2 background,
US3 palettes, US4 effects) for independent implementation and verification.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no incomplete dependencies)
- **[Story]**: US1/US2/US3/US4 — maps to spec.md user stories
- Exact file paths included

## Path Conventions

Single embedded-ROM project: `src/`, `include/`, `res/` at repo root (per plan.md).

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Wire the new/revived modules into the build and create header skeletons.

- [X] T001 Re-enable `src/sprite.c` and `src/background.c` and add `src/palette.c`, `src/effects.c` to the `SRC_FILES` list in `CMakeLists.txt`; update the "unused — excluded" comment (lines 12, 36–37) to reflect they are now built.
- [X] T002 [P] Create header skeleton `include/palette.h` (guard `PALETTE_H`, `#include <gb/gb.h>`/`<gb/cgb.h>`/`<stdint.h>`, `PALETTE_NONE 0xFFu`) per `contracts/palette.md`.
- [X] T003 [P] Create header skeleton `include/effects.h` (guard `EFFECTS_H`, effect type/mode enums `EFFECT_NONE/BLINK/FADE/SCROLL/ANIM`, `ANIM_LOOP/ANIM_ONCE`, `MAX_EFFECTS 4`) per `contracts/effects.md`.
- [X] T004 Configure & build a baseline ROM (`cmake --preset ninja-gbdk` then `cmake --build --preset build-rom`) to confirm the empty modules link green before logic is added.

**Checkpoint**: Build is green with the four modules linked (stub bodies allowed).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Init wiring + the palette module that US2/US3/US4-fade depend on.

**⚠️ CRITICAL**: No user-story work begins until this phase is complete.

- [X] T005 [P] Implement palette module `src/palette.c` + finalize `include/palette.h`: `bkg_slots_used`/`obj_slots_used` bitmaps, `palette_init`, `palette_set_bkg`, `palette_set_obj`, `palette_alloc_bkg`, `palette_free`, `palette_dmg_grayscale`. Guard every color write with `_cpu == CGB_TYPE`; follow `examples/colorbar` set_*_palette idiom (FR-009/010/011).
- [X] T006 Add init calls in `src/game_system.c::game_system_init`: `sprite_init()`, `bg_init()`, `palette_init()`, `effects_init()`. Preserve the existing one-time `set_sprite_data` VRAM load and font/palette setup (do not move it).
- [X] T007 Add `effects_tick()` invocation once per frame in the main loop / `wait_vbl_done` path (identify the loop via the gameplay/main flow); guard so a no-op when no effects active.

**Checkpoint**: ROM boots; init + per-frame tick wired; palette module callable.

---

## Phase 3: User Story 1 - Managed sprite placement (Priority: P1) 🎯 MVP

**Goal**: Allocate/free sprites by handle with VRAM tile reuse (ref-counted) and OAM-slot allocation within the sub-40 range, never exceeding the 88-tile budget.

**Independent Test**: In BGB VRAM/OAM viewer, allocate sprites sharing a graphic (tiles loaded once), exhaust the budget (alloc returns NULL, nothing overwritten), free and confirm release (quickstart Scenario 1).

- [X] T008 [US1] Rewrite `include/sprite.h`: add `graphic_id` to `Sprite`; new `sprite_alloc(uint8_t graphic_id, const uint8_t* tiles, uint8_t tile_count)` signature; declare `TileAlloc` table constants (`MAX_SPRITES`, `MAX_TILE_ALLOCS`) per `data-model.md` + `contracts/sprite_manager.md`.
- [X] T009 [US1] Implement the `TileAlloc` table in `src/sprite.c`: lookup-by-`graphic_id`, free-gap finder within the 88-tile budget (start above tile 167 / reuse documented free ranges), `set_sprite_data` once, ref_count inc/dec, release at zero (FR-002/004/005).
- [X] T010 [US1] Implement OAM allocation in `src/sprite.c`: free-slot cursor/bitmap over indices < 40 that reserves the live player + `NPC_OAM_BASE` ranges; `sprite_alloc`/`sprite_free` returning `NULL` on exhaustion (FR-003). Use shadow OAM (`move_sprite`/`hide_sprites_range`) only.
- [X] T011 [US1] Implement remaining `src/sprite.c` API: `sprite_init`, `sprite_set_pos`, `sprite_set_visible`, `sprite_set_tiles` (2×2 safe, id<40), `sprite_set_palette` (0–7), `sprite_update_all`, `sprite_clear_all` per `contracts/sprite_manager.md`.
- [ ] T012 [US1] Build green and verify quickstart Scenario 1 in BGB (tile reuse, budget refusal, free/release). Record OAM/VRAM viewer observations.

**Checkpoint**: Sprite manager fully functional and independently verifiable — MVP.

---

## Phase 4: User Story 2 - Background layer separate from UI/text (Priority: P2)

**Goal**: A background whose tile range + palettes are disjoint from font/UI tiles, coexisting with the dialogue/text layer, updated only outside Mode 3.

**Independent Test**: Show a background + dialogue typewriter simultaneously; text legible, font tiles untouched, background intact after dismiss (quickstart Scenario 2).

- [X] T013 [US2] Rewrite `include/background.h`: keep `background_t`; define BG-content tile range disjoint from `FONT_OFFSET`/UI tiles; declare `bg_clear`; keep scroll hooks for US4.
- [X] T014 [US2] Implement `src/background.c`: `bg_init`, `bg_load` (tiles into content range, CGB attrmap via `VBK_REG` bank 1 only when `_cpu == CGB_TYPE`, writes during `wait_vbl_done`), `bg_set_visible`, `bg_set_tile`, `bg_clear` (clears only content range) per `contracts/background_layer.md` (FR-006/007/008).
- [X] T015 [US2] Wire `bg_set_palette` to delegate to the palette module (`palette_set_bkg`/`palette_alloc_bkg`); DMG-safe.
- [ ] T016 [US2] Build green and verify quickstart Scenario 2 in BGB: background + dialogue box coexist, font tiles disjoint in VRAM viewer.

**Checkpoint**: Background layer works alongside the existing UI/text layer.

---

## Phase 5: User Story 3 - Dynamic CGB palettes with DMG fallback (Priority: P2)

**Goal**: Runtime BG/OBJ palette assignment on CGB (slots 0–7), grayscale fallback on DMG, limit reported not corrupted.

**Independent Test**: CGB — assign palettes (colors show), request a 9th (returns `PALETTE_NONE`); DMG mode — grayscale, no crash (quickstart Scenario 3).

> Palette module implemented in Phase 2 (T005). This story exercises and proves it end-to-end via sprite + background content.

- [X] T017 [US3] Apply runtime palette assignment paths: `sprite_set_palette` honors palette slots from `palette_alloc_obj`/`palette_set_obj`; confirm BG path via `bg_set_palette`. Add over-limit handling surfacing `PALETTE_NONE` to callers (FR-011).
- [X] T018 [US3] Implement/verify DMG fallback: `palette_dmg_grayscale` invoked on `_cpu != CGB_TYPE` paths so sprite + background content renders in grayscale (FR-010), reusing the `0xE4` default-palette convention from `utils.c`.
- [ ] T019 [US3] Build green and verify quickstart Scenario 3 in BGB CGB mode AND DMG mode (colors, 9th-palette refusal, grayscale, no garble).

**Checkpoint**: Color works on CGB and degrades gracefully on DMG.

---

## Phase 6: User Story 4 - Reusable visual effects, bounded WRAM (Priority: P3)

**Goal**: Blink, fade, scroll, frame animation over a single fixed `EffectState[MAX_EFFECTS]` array; no WRAM growth beyond it; invalid timing clamped; freed targets detach.

**Independent Test**: Each effect produces its visible behavior in BGB; freeing a targeted sprite detaches the effect; build map shows effects WRAM = `EffectState[MAX_EFFECTS]` only (quickstart Scenario 4).

- [X] T020 [US4] Implement core engine in `src/effects.c` + finalize `include/effects.h`: `static EffectState effects[MAX_EFFECTS]`, `effects_init`, `effects_tick` (advance active slots, clamp `period≥1`, detach freed targets — FR-016/017/018), `effect_stop`.
- [X] T021 [P] [US4] Implement `effect_blink(sprite_id, period)` toggling visibility via `sprite_set_visible`, reusing the 30-frame cadence idiom from `title_screen.c::draw_blink_line` (FR-012).
- [X] T022 [P] [US4] Implement `effect_fade(direction, frames)` delegating to `utils.c` `fade_to_black`/`fade_from_black` (DMG-safe LUT), `frames` clamped ≥1 (FR-013).
- [X] T023 [P] [US4] Implement `effect_scroll(bg_id, dx, dy, period)` driving `bg_set_scroll`/`SCX/SCY` per `examples/galaxy`, V-Blank timing — no tearing (FR-014).
- [X] T024 [P] [US4] Implement `effect_anim(sprite_id, first_tile, frame_count, period, mode)` cycling tiles via `sprite_set_tiles`, `ANIM_LOOP`/`ANIM_ONCE` (FR-015).
- [ ] T025 [US4] Build green and verify quickstart Scenario 4 in BGB (all four effects, detach-on-free) and inspect the `.map`/`.noi` build map to confirm effects WRAM = `EffectState[MAX_EFFECTS]` only (SC-007).

**Checkpoint**: All four effects functional within the fixed WRAM budget.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Verification, budget reporting, docs, index refresh.

- [ ] T026 Run full `quickstart.md` (Scenarios 1–4) in BGB; re-check Scenario 3 + fade in DMG mode. Record emulator, scenarios exercised, results (Principle II).
- [X] T027 Capture the final WRAM figure from the build map and confirm it matches the `data-model.md` budget (~446 B); tune `MAX_SPRITES`/`MAX_TILE_ALLOCS`/`MAX_EFFECTS` if over budget. Report per-feature RAM cost (Principle IV).
  <!-- Result: sprite.c 296B + background.c 28B + palette.c 2B + effects.c 44B = 370B total. Under 446B budget. No tuning needed. -Wl-m added to CMakeLists.txt to emit rom/*.map. -->
- [X] T028 [P] Update `docs/Roadmap.md` VRAM-budget note and `README.md` Graphics section to reflect the implemented sprite manager / background / palette / effects layer.
- [X] T029 [P] Refresh the `ccc` index (`ccc index`) after all code changes (Principle V).

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately.
- **Foundational (Phase 2)**: Depends on Setup. BLOCKS all user stories.
- **User Stories (Phase 3–6)**: All depend on Foundational.
  - US1 (P1) independent. US2 (P2) and US3 (P2) depend on the palette module (Phase 2). US4 (P3) depends on US1 (blink/anim target sprites) and US2 (scroll target background).
- **Polish (Phase 7)**: Depends on the desired user stories being complete.

### User Story Dependencies

- **US1 (P1)**: After Foundational. No dependency on other stories.
- **US2 (P2)**: After Foundational (palette module). Independently testable.
- **US3 (P2)**: After Foundational; exercises palette via US1/US2 content but verifiable on its own.
- **US4 (P3)**: After US1 + US2 (needs sprite + background targets).

### Within Each User Story

- Header/struct before implementation; implementation before BGB verification.
- Each story ends with a green build + quickstart verification task.

### Parallel Opportunities

- T002, T003 (header skeletons) in parallel.
- T021–T024 (blink/fade/scroll/anim) in parallel — distinct functions in `effects.c` once T020 lands (coordinate same-file edits or split by section).
- T028, T029 (docs / index refresh) in parallel.
- US2 and US3 can proceed in parallel after Foundational (different files: `background.c` vs palette exercise paths).

---

## Parallel Example: Phase 1 Setup

```bash
Task: "Create header skeleton include/palette.h per contracts/palette.md"
Task: "Create header skeleton include/effects.h per contracts/effects.md"
```

---

## Implementation Strategy

### MVP First (User Story 1 only)

1. Phase 1 Setup → 2. Phase 2 Foundational → 3. Phase 3 US1 → STOP & verify the
   sprite manager in BGB (quickstart Scenario 1). Sprite management alone is a
   usable MVP.

### Incremental Delivery

1. Setup + Foundational → foundation ready.
2. US1 → verify → MVP.
3. US2 → verify (background + dialogue).
4. US3 → verify (CGB + DMG palettes).
5. US4 → verify (effects + WRAM map).
6. Polish → final verification + budget report.

---

## Notes

- No automated tests — verification tasks (T012, T016, T019, T025, T026) are manual BGB/SameBoy per `quickstart.md`.
- [P] = different files, no incomplete dependency.
- Commit after each task or logical group (Conventional Commits; confirm before committing).
- Respect constitution: VRAM writes V-Blank/H-Blank only, OAM<40, palette 0–7, single-module banks, report WRAM cost.
- Stop at any checkpoint to validate a story independently.
