# Quickstart: Validating VRAM & Scene Visual Polish

There is no automated test suite (Constitution Principle II). Validation is a
manual emulator pass. Every scenario below must be observed and reported.

## Prerequisites

- GBDK-2020 at `C:/gbdk` (`GBDK_ROOT`)
- BGB (preferred — has the VRAM/OAM/Palette viewers) or SameBoy

## Build & run

```bash
cmake --preset ninja-gbdk          # configure
cmake --build --preset build-rom   # build → rom/gbc-c-starter-kit.gbc
cmake --build build --target run   # launch in BGB
```

The build must be green before any scenario is judged.

## Emulator setup

Open **BGB → right-click → Debug → VRAM viewer**, and keep these tabs handy:

- **Tiles** — verify tile-range ownership
- **OAM** — verify sprite slot/geometry
- **Palettes** — verify per-actor palette slots

## Scenario 0 — Font base tile (prerequisite for everything else)

Covers R0. Do this first; it gates the other scenarios.

1. Boot the ROM, open the VRAM viewer **Tiles** tab.
2. Locate the ASCII font block (` !"#$%&'` … `xyz{|}~`).
3. Note the tile index of the font's first glyph (space).

**Expected**: the font occupies 96 contiguous tiles starting at
`first_tile` (research asserts `0`), the world tileset sits at 128–131, and the
title logo occupies tiles in 96–127. Confirm no range overlaps another.

## Scenario 1 — Dialogue box border (US1, FR-001/FR-002)

1. Start a new game and let the intro cutscene run to the dialogue.
2. Watch the box slide in and the typewriter run.

**Expected**:

- The frame has four corners, top/bottom rules, **and visible left/right
  vertical edges** (the current build shows blank sides).
- No literal `+`/`-`/`|` placeholder look where a border glyph belongs.
- The border stays stable while text types out.
- Text remains inside the frame.

**DMG check**: relaunch with BGB forced to DMG — the border still renders.

## Scenario 2 — Gameplay actors (US2, FR-003/FR-004)

1. Enter gameplay. Move until the player, treasure, and hazard are on screen
   together.
2. Open the **OAM** and **Palettes** tabs.

**Expected**:

- Treasure and hazard render as their intended graphics — **not** the red/pink
  and blue garbage blocks seen today (the 32-byte over-read, R3).
- The player renders at correct 16×16 proportions using **2** OAM entries, not
  a stretched 2×2 block of four 8×16 sprites (R4).
- Player, treasure, and hazard each sit on a **different** OBJ palette slot
  (0, 2, 3 per `data-model.md` E4) — no actor borrowing the player's colours.
- All OAM indices are < 40, and objects (0–19) never collide with the
  player (20–21).

## Scenario 3 — Win / game-over screens (US2, FR-005)

1. Collect the treasure to reach the win screen (or lose all lives for game
   over).

**Expected**: a clean solid background. The full-screen field of `'@'`
characters visible today (`Screenshot ...121646.png`) is gone — this is the
`clear_screen()` fix from R0. Title, treasure message, score, and the
`PRESS START` prompt are all legible.

## Scenario 4 — Credits non-regression (US2, FR-006)

1. Continue past the win screen into the credits roll.

**Expected**: the scroll renders at least as correctly as before the change.
Re-check the beige banding noted as an open item in `research.md`; if it
persists after R0/R5, raise it separately.

## Scenario 5 — Title screen logo (US3, FR-007)

1. Return to the title screen.

**Expected**: a dedicated logo graphic. The garbled `gbc@p` / `PNRNP` rendering
(`Screenshot ...121923.png`) is gone. In the **Tiles** tab, the logo occupies
BG tiles 96–127 and has not displaced the font (0–95) or tileset (128–131).

## Scenario 6 — Options screen layout (US3, FR-008/FR-009)

1. Press SELECT to open options. Cycle every row through **all** its values.

**Expected**:

- Every row fits within the 20-column screen. `DIFFICULTY: NORMAL` no longer
  wraps `AL` onto the next line.
- The build line (`GBC Prototype v0.2.0`) fits on one row.
- No character is drawn off the right edge at any value.
- Text uses one consistent palette across all rows on CGB.
- Switch language to IT and repeat — longer strings truncate, never wrap.

## Scenario 7 — Splash leftover sprite (FR-010)

1. Reset the ROM and watch both splash screens from a cold boot, then reset
   again *from gameplay* so sprites are live when the splash re-enters.

**Expected**: no stray sprite block at the top-left
(`Screenshot ...121121.png`) — the shadow OAM is cleared before drawing (R8).

## Scenario 8 — Distinct cutscene actors (US4, FR-012)

1. Watch the intro cutscene.

**Expected**: the lead and the buddy are visually **distinct** from each other
and from the gameplay player (today both are the same Alex sprite). In the
**Tiles** tab, the lead draws from base 128 and the buddy from base 148; in
**Palettes**, they occupy different OBJ slots.

## Scenario 9 — Full-flow transition sweep (FR-010)

Run the whole flow in one sitting:

`splash → intro → title → options → title → gameplay → win → credits → title`

**Expected**: at every transition, no stale tiles, sprites, or palette colours
carry over from the previous screen.

## Reporting

Per Constitution Principle II, state the emulator used, which scenarios were
exercised, and the observed result. Never report "works" for a scenario that
was not actually run.

## Budget check

Confirm against `data-model.md`: net WRAM growth is **+1 byte** (the cached
font base tile). Report the figure with the completion summary.
