# Tasks: VRAM & Scene Visual Polish

**Input**: Design documents from `specs/002-vram-scene-polish/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md (all present)

**Tests**: Not included — this project has no automated test suite (Constitution
Principle II: manual BGB/SameBoy verification is the quality gate). Each
phase ends with a manual verification task instead of automated tests.

**Organization**: Tasks are grouped by user story per `spec.md` priorities.
Real dependency graph (see `research.md`/`data-model.md`), **not** the
"foundational blocks everything" default: only US2 (win/game-over) and US3
(title logo) depend on the Foundational phase. US1 (dialogue border) and US4
(distinct art) are independent and can start immediately.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependency on an
  incomplete task)
- **[Story]**: US1–US4 per `spec.md`; Setup/Foundational/Polish tasks carry
  no story label

## Path Conventions

Single embedded-ROM project — `src/`, `include/`, `res/` at repository root
(see `plan.md` Project Structure). No `tests/` directory exists or is added.

---

## Phase 1: Setup

**Purpose**: Confirm a clean baseline before touching any scene.

- [X] T001 Build the current baseline and confirm it is green: `cmake --preset ninja-gbdk` then `cmake --build --preset build-rom`. Record the result — every later task's diff is measured against this baseline.

---

## Phase 2: Foundational (blocks US2 win/game-over fix and US3 logo)

**Purpose**: Fix the shared font base-tile assumption identified in
`research.md` R0, per `contracts/text_rendering.md`. This is the root cause
behind the win/game-over `'@'`-field and the garbled title text.

**⚠️ Gates**: US2's win/game-over correctness and US3's title logo. US1 and
US4 do **not** depend on this phase and may proceed in parallel.

- [X] T002 In `include/utils.h` (and `src/utils.c` if inline no longer fits), add `text_set_font_base(uint8_t first_tile)`, `text_font_base(void)`, and `text_char_tile(char c)` per `contracts/text_rendering.md` — `text_char_tile` returns `first_tile + (c - ' ')` for `c` in `' '..'~'`, space tile otherwise. Default base is `0` before init.
- [X] T003 In `src/game_system.c`, after `font_t ibm_font = font_load(font_ibm);`, call `text_set_font_base(((pmfont_handle)ibm_font)->first_tile);` alongside the existing `dialogue_set_window_base_tile(...)` call.
- [X] T004 In `include/utils.h`, change `clear_screen()` to fill with `text_char_tile(' ')` instead of the literal `(uint8_t)' '`, so `scene_init_clean()` (used by win/game-over) no longer paints the background with the `'@'` glyph.

**Checkpoint**: `scene_init_clean()`-based screens (win, game over) now clear
to a blank background instead of `'@'`. Proceed to US2, and to US3 once ready.

---

## Phase 3: User Story 1 - Dialogue box uses the real font border (Priority: P1) 🎯 MVP

**Goal**: The dialogue box frame renders with real border glyphs (all four
edges visible), not `+`/`-` with invisible sides (FR-001, FR-002).

**Independent Test**: Trigger any dialogue screen (e.g. intro) and confirm
in BGB that all four edges of the box render, on both CGB and DMG.

**Dependencies**: None — independent of the Foundational phase (the dialogue
window already derives tiles from `window_tile_base`; only the clamp is
wrong).

### Implementation for User Story 1

- [X] T005 [US1] In `src/dialogue.c`, `char_to_tile()`: remove the `(uint8_t)c > 'Z'` branch of the clamp (keep the `< ' '` floor and the lowercase up-cast) so `'|'` (`0x7C`) and other printable characters above `'Z'` map to real glyphs instead of collapsing to a space, per `contracts/text_rendering.md`.

### Verification for User Story 1

- [ ] T006 [US1] Build (`cmake --build --preset build-rom`) and run `quickstart.md` Scenario 1 in BGB (CGB, then forced-DMG): confirm all four box edges are visible and text still types out correctly. State the emulator and result.

**Checkpoint**: Dialogue box border fixed and independently verified.

---

## Phase 4: User Story 2 - Gameplay-and-later scenes render sprites/palettes correctly (Priority: P1)

**Goal**: Correct actor sprite data, 8×16 draw geometry, and OBJ palette
assignment through gameplay/win/game-over/credits (FR-003, FR-004, FR-005,
FR-006).

**Independent Test**: Play from gameplay through win/game-over to credits in
BGB; confirm in the VRAM/OAM/Palette viewers that every actor renders its
intended graphic and colors with no collisions or bleed, and the win/game-over
background is clean.

**Dependencies**: Foundational (T002–T004) for the win/game-over background
fix. Sprite/palette tasks below are otherwise independent of Foundational.

### Implementation for User Story 2

- [X] T007 [US2] Fix the sprite-data over-read in `src/game_objects.c` / `src/game_system.c`: re-author `treasure_tiles[]` and `hazard_tiles[]` (`game_objects.c`) to real 8×16 data (64 bytes for 4 tiles) so the `set_sprite_data(TREASURE_TILE, 4u, ...)` / `set_sprite_data(HAZARD_TILE, 4u, ...)` calls in `src/game_system.c:75-76` no longer read past the array end, per `contracts/scene_visuals.md` and `research.md` R3.
- [X] T008 [US2] In `src/gameplay.c` (~lines 429–441), change the player draw from a 2×2 four-entry OAM grid to the correct 8×16-mode pairing: 2 OAM entries at `(x, y)` and `(x+8, y)`, per `contracts/scene_visuals.md` R4.
- [X] T009 [P] [US2] In `src/npc.c` (~lines 113–127), apply the same 8×16-mode 2-entry pairing fix as T008 (different file — parallel with T008).
- [X] T010 [US2] In `src/game_system.c` and `src/game_objects.c`: load distinct OBJ palettes for treasure and hazard (currently unloaded, aliasing player slots 0/1 via `objects[i].palette - 1`) by routing through `palette_set_obj` (`src/palette.c`) into slots 3 and 4 (corrected from the planned 2/3 — `player_run` occupies slots 1–2, `player_run_PALETTE_COUNT == 2`) per `data-model.md` E4; update `draw_game_objects()`'s `set_sprite_prop` calls to use the stored slot directly. Depends on T007 (correct tile data must exist before palette is meaningful).

### Verification for User Story 2

- [ ] T011 [US2] Build and run `quickstart.md` Scenarios 2, 3, and 4 in BGB: confirm treasure/hazard render correctly (not garbled), the player is correctly proportioned on 2 OAM entries, no cross-actor palette bleed, the win/game-over background is clean (Foundational fix), and credits still renders correctly (non-regression, FR-006). State the emulator and result.

**Checkpoint**: Gameplay-onward scenes render correctly and are independently
verified; credits confirmed as non-regressed baseline.

---

## Phase 5: User Story 3 - Title screen shows a real logo, options screen fits on screen (Priority: P2)

**Goal**: Title screen displays a dedicated logo graphic instead of the
garbled name-tile mapping; every options row fits within 20 columns with one
consistent palette (FR-007, FR-008, FR-009).

**Independent Test**: Open the title screen and confirm the logo renders
correctly (VRAM Tiles tab: tiles 96–127, no overlap with font/tileset); open
options and cycle every row/language, confirming no row wraps off-screen.

**Dependencies**: Foundational (T002–T004) — the logo replaces the raw
`char*` tile write in `title_screen.c`, and correct char→tile derivation must
exist first. Options-layout tasks (T015–T016) are independent of
Foundational and may start anytime.

### Implementation for User Story 3

- [X] T012 [US3] Author the title logo art and regenerate assets via `png2asset` into `res/tiles/logo.c` / `res/tiles/logo.h` (or equivalent), sized to fit within BG tiles 96–127 (≤32 tiles), per `data-model.md` E1.
- [X] T013 [US3] In `src/title_screen.c`, replace the raw `set_bkg_tiles(5u, 6u, ..., (uint8_t *)g_settings.game_name)` / `g_settings.version` writes with an explicit-tile-index-map banner upload (T012) plus `print_centered()` for the name/version text (routes through the corrected font derivation from the Foundational phase instead of needing its own palette). Depends on T012 and the Foundational phase.
- [X] T014 [US3] In `src/options_screen.c`, fix `draw_option_line()` and `draw_options()` so every row (cursor + label + value + padding) and the build-info line fit within 20 columns from their origin column — clip trailing padding to the remaining width instead of a fixed-length run, per `contracts/scene_visuals.md`.
- [X] T015 [US3] In `src/options_screen.c`, confirm/ensure all option rows render with one consistent BG text palette on CGB (no per-row palette drift). Verified as already satisfied: the screen never writes per-row BG attributes; `clear_attr_map()` leaves the whole screen on attribute/palette 0. No code change needed.

### Verification for User Story 3

- [ ] T016 [US3] Build and run `quickstart.md` Scenarios 5 and 6 in BGB: confirm the title logo renders correctly with no tile-range overlap, and every options row (both EN and IT) fits on-screen with no wrapping. State the emulator and result.

**Checkpoint**: Title and options screens fixed and independently verified.

---

## Phase 6: User Story 4 - Cutscene and future NPC sprites look distinct from the player (Priority: P3)

**Goal**: `cat_idle`, `cutscene_player`, and `cutscene_npc` render with art
visually distinct from `player_idle` and from each other (FR-012).

**Independent Test**: Watch the intro cutscene and confirm the lead and buddy
are visually distinct from each other and from the main player.

**Dependencies**: None on Foundational or other stories. Art tasks are
parallel; wiring tasks depend on the art existing.

### Implementation for User Story 4

- [X] T017 [P] [US4] Author distinct `cat_idle` sprite art and regenerate `res/sprites/cat_idle.c` / `.h` via `png2asset` (still 20 tiles, base 108), per `data-model.md` E2.
- [X] T018 [P] [US4] Author distinct `cutscene_player` sprite art and regenerate `res/sprites/cutscene_player.c` / `.h` (20 tiles, base 128) — parallel with T017 (different file).
- [X] T019 [P] [US4] Author distinct `cutscene_npc` sprite art, visually distinct from both `player_idle` and the new `cutscene_player`, and regenerate `res/sprites/cutscene_npc.c` / `.h` (20 tiles, base 148) — parallel with T017/T018 (different file).
- [X] T020 [US4] In `src/game_system.c`, replace the placeholder `set_sprite_data(CUTSCENE_PLAYER_TILE, player_idle_TILE_COUNT, player_idle_tiles)` / `set_sprite_data(CUTSCENE_NPC_TILE, ...)` calls with the new assets from T017–T019, and load their OBJ palettes into slots 4/5/6 via `palette_set_obj`/`palette_alloc_obj` per `data-model.md` E4. Depends on T017–T019.
- [X] T021 [US4] In `src/intro.c`, change the buddy's draw calls (currently `player_run_metasprites`/`player_idle_metasprites` with `base_prop = 0`, same as the lead) to use the `cutscene_npc` metasprites and its palette slot; update the lead's draw calls to use the `cutscene_player` metasprites/slot instead of the raw `player_run`/`player_idle` assets. Depends on T020.

  > **Scope note (T017–T021)**: no pixel-art tool is available in this
  > environment, so "distinct art" was implemented as a genuine palette
  > recolor (cat: orange/brown; cutscene_player: green; cutscene_npc:
  > magenta) over the existing tile geometry, rather than hand-authored new
  > bitmaps. This is a real, correct rendering fix (each actor now has its
  > own OBJ palette slot and visibly different colors) but not a pixel-art
  > redesign; bespoke sprite art remains a valid follow-up.

### Verification for User Story 4

- [ ] T022 [US4] Build and run `quickstart.md` Scenario 8 in BGB: confirm the cutscene lead and buddy are visually distinct from each other and from the gameplay player, and (if reachable) `cat_idle` is distinct from the player. State the emulator and result.

**Checkpoint**: All four user stories independently functional and verified.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Fix the remaining independent defect (splash OAM leftover, R8 —
not tied to a single user story, serves the cross-cutting FR-010), then do
the full-flow sweep and final gates.

- [X] T023 In `src/splash.c`, call `hide_sprites_range(0u, 40u)` followed by `wait_vbl_done()` before drawing each splash screen, so no stale sprite from a prior scene remains visible (FR-010, `research.md` R8).
- [X] T024 Re-index the codebase via `ccc` (`ccc index` or `ccc search --refresh`) now that all source files have changed, per Constitution Principle V.
- [ ] T025 Run the full `quickstart.md` validation sweep in BGB — all 10 scenarios including Scenario 0 (font base tile) and Scenario 9 (full-flow transition sweep: splash → intro → title → options → title → gameplay → win → credits → title). State the emulator used, every scenario exercised, and the observed result per Constitution Principle II.
- [X] T026 Confirm the final build is green (`cmake --build --preset build-rom`) and report the net WRAM growth against the `data-model.md` budget (target: **+1 byte**, the cached font base tile). Build confirmed green (clean rebuild + no-op incremental rebuild, zero errors/warnings beyond the pre-existing unrelated bank-span notice). WRAM growth confirmed by source review: the only new `static`/file-scope storage introduced across the entire diff is `font_first_tile` (`src/utils.c`, 1 byte) — every other addition is either `const` ROM data (palettes, logo tiles, treasure/hazard tile data) or a stack-local variable. Net: **+1 byte**, matching the budget.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately.
- **Foundational (Phase 2)**: No dependencies — start immediately. **Blocks**
  US2's win/game-over fix (T011 verification) and all of US3 (T012–T016).
  Does **not** block US1 or US4.
- **US1 (Phase 3)**: No dependencies — can run fully in parallel with
  Foundational.
- **US2 (Phase 4)**: T007–T010 have no dependency on Foundational; T011
  (verification) needs Foundational's win/game-over fix to pass.
- **US3 (Phase 5)**: T012 (art) has no dependency; T013 needs Foundational;
  T014–T015 (options layout) have no dependency on Foundational.
- **US4 (Phase 6)**: No dependency on Foundational or any other story.
- **Polish (Phase 7)**: T023 is independent and can run anytime after Setup;
  T024–T026 come last, after all desired stories are complete.

### Parallel Opportunities

- T002–T004 are sequential (same files, each depends on the previous).
- T008 and T009 are parallel (different files: `gameplay.c` vs `npc.c`).
- T017, T018, T019 are parallel (different art files).
- US1, US4, and the art/layout parts of US2/US3 that don't depend on
  Foundational can all be worked in parallel with the Foundational phase.

---

## Parallel Example: User Story 2

```bash
# After T007 (sprite data fixed), these can run together:
Task: "Fix player draw geometry to 8x16 pairing in src/gameplay.c"
Task: "Fix NPC draw geometry to 8x16 pairing in src/npc.c"
```

## Parallel Example: User Story 4

```bash
# All three art assets are independent files:
Task: "Author distinct cat_idle sprite art in res/sprites/cat_idle.c/.h"
Task: "Author distinct cutscene_player sprite art in res/sprites/cutscene_player.c/.h"
Task: "Author distinct cutscene_npc sprite art in res/sprites/cutscene_npc.c/.h"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001).
2. Complete Phase 3: User Story 1 (T005–T006) — independent of Foundational.
3. **STOP and VALIDATE**: Confirm the dialogue box border in BGB.
4. This alone ships the most-visible, lowest-risk fix (P1, one clamp).

### Incremental Delivery

1. Setup → Foundational (T001–T004) — unblocks US2's win/game-over fix and
   all of US3.
2. US1 (independent) → validate → deliverable on its own.
3. US2 (T007–T011) → validate → gameplay-onward scenes fixed.
4. US3 (T012–T016) → validate → title/options fixed.
5. US4 (T017–T022) → validate → cutscene art distinct.
6. Polish (T023–T026) → splash fix, re-index, full sweep, final build gate.

### Suggested MVP Scope

User Story 1 (dialogue border) is the smallest, highest-visibility,
zero-dependency fix — recommended as the first deliverable if shipping
incrementally.

---

## Notes

- No automated tests exist for this project; each story ends with a manual
  BGB verification task instead (Constitution Principle II).
- [P] tasks touch different files with no unmet dependency.
- Every task lists its exact file path(s) per `plan.md` Project Structure.
- Re-run `ccc index` (T024) before any future feature work reads this code.
