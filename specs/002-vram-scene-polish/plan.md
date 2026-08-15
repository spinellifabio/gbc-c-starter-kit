# Implementation Plan: VRAM & Scene Visual Polish

**Branch**: `master` (no feature branch created) | **Date**: 2026-08-15 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/002-vram-scene-polish/spec.md`

## Summary

Correct VRAM, sprite, palette, and layout usage across every screen implemented
so far, and add the two art assets confirmed in the clarification session
(title logo; distinct `cat_idle` / `cutscene_player` / `cutscene_npc`).

Phase 0 research identified a **single root cause behind most of the
"compromised" scenes**: GBDK's `font_ibm` loads at `first_tile = 0`, but two
call sites assume ASCII equals the tile index (`first_tile == 32`).
`clear_screen()` therefore fills the background with tile 32 — the `'@'` glyph —
which is exactly the `@`-field covering the win and game-over screens, and
`title_screen.c` writes a raw `char*` as tile indices, producing the garbled
`gbc@p` title. Fixing the char→tile derivation repairs both at once. The
remaining defects are independent and individually small: a 32-byte sprite-data
over-read corrupting treasure/hazard, a 2×2 OAM layout that contradicts
`SPRITES_8x16`, object palettes aliasing the player's slots, options rows
overflowing 20 columns, an unhidden shadow OAM on the splash screen, and the
dialogue box's `> 'Z'` clamp silently turning `'|'` into a space so the text box
has no vertical edges.

Approach: fix the shared text/tile derivation first (it gates the scene
scenarios), then the per-scene sprite/palette/layout defects, then add the new
art. Net WRAM growth is **+1 byte**.

## Technical Context

**Language/Version**: C99 (SDCC via GBDK-2020 `lcc`), no heap, no `<stdbool.h>`

**Primary Dependencies**: GBDK-2020 (`GBDK_ROOT=C:/gbdk`), `png2asset` for sprite/tile assets

**Storage**: N/A (SRAM save system exists but is untouched by this feature)

**Testing**: Manual verification in BGB (VRAM/OAM/Palette viewers) or SameBoy — no automated suite

**Target Platform**: Game Boy Color (CGB), with graceful DMG degradation via `_cpu == CGB_TYPE`

**Project Type**: Single-target embedded ROM (Z80/GBZ80)

**Performance Goals**: 60 FPS; VRAM/palette writes only outside LCD Mode 3

**Constraints**: BG tiles — font 0–95, tileset 128–131, logo ≤32 tiles in 96–127; sprite tiles 168/256 used, 88 free; OAM index < 40; palette slots 0–7; net WRAM growth +1 byte; single-module `#pragma bank` only

**Scale/Scope**: 9 existing `.c` files touched, 4 new/re-authored art assets, no new modules

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Requirement | Status |
|-----------|-------------|--------|
| I. Code Quality Discipline | C99, `static` privates, `const` ROM data, explicit narrowing casts, unsigned HW literals, follow `examples/` idiom | **PASS** — all changes are edits to existing files in their established style; no new idioms |
| II. Manual Verification & Testing | Build green + BGB/SameBoy verification stated; DMG degradation checked | **PASS** — `quickstart.md` defines 10 scenarios incl. an explicit DMG check; Principle II reporting rule restated there |
| III. UX Consistency | Keep the screen flow; use shared text helpers (`print_centered`, dialogue, `FONT_OFFSET`); no ad-hoc tile writes | **PASS** — the plan *removes* the two ad-hoc tile writes (`clear_screen` literal, `title_screen` raw string) and routes them through the font-derived helper |
| IV. Memory & Resource Optimization | No net WRAM growth unless required and stated; reuse slots; ROM `const`; single-module banks | **PASS** — +1 byte, stated in `data-model.md`; logo uses previously-unused BG tiles; new art reuses already-reserved sprite ranges; player drops from 4 OAM entries to 2; no banking change |
| V. CCC-First Code Search | `ccc` before grep/read; refine with identifiers; re-index after changes | **PASS** — every finding in `research.md` came from `ccc` searches followed by narrowed reads; re-index is a task-phase step |

**Hardware constraints**: VRAM/palette writes during `DISPLAY_OFF` or
`wait_vbl_done()` only; OAM < 40; palette 0–7. No new banking.

**Initial gate**: PASS — no violations. Complexity Tracking not required.

**Post-design re-check**: PASS — Phase 1 introduces no new module, no new WRAM
structure beyond the single cached base tile, and no new banking. The one
judgement call (adding new art assets) was an explicit user clarification, and
it consumes only previously-unused VRAM.

## Project Structure

### Documentation (this feature)

```text
specs/002-vram-scene-polish/
├── plan.md              # This file
├── research.md          # Phase 0 — root-cause analysis (R0–R8)
├── data-model.md        # Phase 1 — VRAM/OAM/palette resource maps (E1–E5)
├── quickstart.md        # Phase 1 — 10 manual validation scenarios
├── contracts/
│   ├── text_rendering.md    # Font base-tile derivation + dialogue border
│   └── scene_visuals.md     # Sprite data, 8×16 geometry, palettes, layout, art
├── checklists/
│   └── requirements.md      # Spec quality checklist (16/16)
└── tasks.md             # Phase 2 output (/speckit-tasks — NOT created here)
```

### Source Code (repository root)

```text
include/
├── utils.h            # TOUCH: clear_screen() uses derived space tile; text_char_tile helper
├── game_objects.h     # TOUCH (maybe): object tile/palette constants
└── world_defs.h       # REFERENCE: FONT_OFFSET (unchanged)

src/
├── utils.c            # TOUCH: font base-tile storage; scene_init_clean unchanged otherwise
├── game_system.c      # TOUCH: pass first_tile to text layer; fix sprite_data sizes; load per-actor palettes; upload logo
├── dialogue.c         # TOUCH: remove the > 'Z' clamp; draw real vertical edges
├── title_screen.c     # TOUCH: draw logo tiles; stop writing raw char* as tile indices
├── options_screen.c   # TOUCH: row layout clipped to 20 columns; consistent palette
├── game_objects.c     # TOUCH: 8×16 draw geometry; palette slots via palette.c; re-authored tile data
├── gameplay.c         # TOUCH: player drawn with 2 OAM entries (8×16), not a 2×2 grid
├── npc.c              # TOUCH: same 8×16 geometry fix
├── intro.c            # TOUCH: draw from CUTSCENE_PLAYER_TILE / CUTSCENE_NPC_TILE with distinct palettes
├── splash.c           # TOUCH: hide shadow OAM before drawing
├── credits.c          # UNTOUCHED: non-regression baseline (FR-006)
└── palette.c          # REUSE: palette_set_obj / palette_alloc_obj (currently unused by these call sites)

res/
├── tiles/
│   └── logo.*         # NEW: title logo, BG tiles 96–127
└── sprites/
    ├── cat_idle.*         # RE-AUTHOR: distinct from player_idle
    ├── cutscene_player.*  # RE-AUTHOR: distinct from player_idle
    └── cutscene_npc.*     # RE-AUTHOR: distinct from cutscene_player
```

**Structure Decision**: The existing flat `include/` + `src/` + `res/` layout is
kept. This feature adds no module — it corrects call sites in place and adds
art under `res/`. `sprite.c` / `background.c` remain excluded from the build
per the Constitution's dead-module rule.

## Implementation Ordering

Dependency-driven, matching spec priorities:

1. **R0 — font base-tile derivation** (`utils.h`/`utils.c`/`game_system.c`).
   Gates Scenarios 3, 5 and unblocks the P1 win/game-over fix. Highest
   value-per-line in the feature.
2. **US1 / R1 — dialogue border** (`dialogue.c`). Independent, P1, one clamp.
3. **US2 / R3–R5 — gameplay actors** (`game_system.c`, `game_objects.c`,
   `gameplay.c`, `npc.c`). Sprite data sizes → 8×16 geometry → palette slots,
   in that order (geometry depends on correct tile data).
4. **R8 — splash OAM** (`splash.c`). Independent, tiny.
5. **US3 / R6 — options layout** (`options_screen.c`). Independent, P2.
6. **US3 / R2 — title logo art** (`res/tiles/`, `title_screen.c`,
   `game_system.c`). Depends on step 1 landing first.
7. **US4 / R7 — distinct actor art** (`res/sprites/`, `intro.c`,
   `game_system.c`). Lowest priority (P3), largest art effort.
8. **Re-index** via `ccc` and run the full `quickstart.md` sweep.

## Complexity Tracking

> No Constitution Check violations. Section intentionally empty.
