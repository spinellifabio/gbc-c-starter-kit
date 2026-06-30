# Implementation Plan: Visual Content Layer

**Branch**: `001-visual-content-layer` | **Date**: 2026-06-29 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/001-visual-content-layer/spec.md`

## Summary

Provide a reusable visual-content layer for the GBC starter kit composed of four
subsystems: (1) a **sprite manager** that hands out sprite handles, reuses
already-resident VRAM tiles via reference counting, and allocates OAM slots from
the free sub-40 range without overflowing the 88-tile budget; (2) a **background
layer** whose tile range and palettes are kept disjoint from the UI/text/font
tiles on the single hardware BG map; (3) **runtime CGB palette assignment** with
graceful DMG grayscale fallback (`_cpu == CGB_TYPE`); and (4) a **fixed-state
effects API** (blink, fade, scroll, basic frame animation) that ticks once per
V-Blank and adds no WRAM beyond a single statically-sized effect-state struct.

Technical approach: revive and rewrite the currently-excluded `src/sprite.c` and
`src/background.c` dead modules into budget-aware managers, add a new
`src/effects.c` + `src/palette.c` (or fold palette helpers into the managers),
and re-enable them in `CMakeLists.txt`. All VRAM writes are gated to V-Blank
(`wait_vbl_done`) or H-Blank per `examples/hblank_copy`; scrolling follows
`examples/galaxy`; palette set assignment follows `examples/colorbar` and
`examples/dscan`. Existing helpers (`fade_to_black`/`fade_from_black` in
`utils.c`, `draw_blink_line` cadence in `title_screen.c`, `npc.c` OAM/metasprite
idiom) are reused rather than reinvented.

## Technical Context

**Language/Version**: C99 (GBDK-2020 / SDCC), no heap, no `<stdbool.h>` (`uint8_t` as bool)

**Primary Dependencies**: GBDK-2020 (`<gb/gb.h>`, `<gb/cgb.h>`, `<gb/metasprites.h>`); existing project modules `utils.c`, `game_system.c`, `npc.c`

**Storage**: N/A (ROM `const` data + fixed WRAM state; SRAM untouched by this feature)

**Testing**: Manual verification in BGB / SameBoy (VRAM + OAM viewers); no automated suite

**Target Platform**: Game Boy Color via GBDK-2020; must boot and degrade gracefully on DMG

**Project Type**: Single-project embedded ROM (flat `src/` + `include/` + `res/`)

**Performance Goals**: 60 FPS; VRAM/OAM writes only outside LCD Mode 3; effects tick O(active effects) per frame with `uint8_t` counters

**Constraints**: VRAM ≤ 256 sprite tiles (168 used, 88 free); OAM index < 40; palette index 0–7; effects API net WRAM growth = one fixed `EffectState` array only; single-module `#pragma bank` rule (no multi-module banks)

**Scale/Scope**: 4 subsystems, ~4 new/revived `.c` modules + headers; reuses existing sprite VRAM layout (player_idle 0–19, player_run 20–99, treasure 100–103, hazard 104–107, placeholders 108–167)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Gate | Status |
|-----------|------|--------|
| I. Code Quality Discipline | C99, no heap/`stdbool`, `static` privates, `const` ROM data, naming table, unsigned HW literals, follow `examples/` idiom | PASS — plan mandates each; managers follow `examples/` samples |
| II. Manual Verification & Testing | Green `build-rom`, ROM boots, CGB+DMG checked, verification stated | PASS — quickstart defines BGB/SameBoy steps incl. DMG mode |
| III. UX Consistency | Reuse `print_centered`/dialogue/`FONT_OFFSET`; do not bypass UI/text layer | PASS — FR-020; background tile range disjoint from font tiles |
| IV. Memory & Resource Optimization | No net WRAM growth unless required + stated; reuse OAM/tiles; LUT/`const`; single-module bank; model on `examples/` | PASS — ref-counted tile reuse, fixed effect-state struct; per-module WRAM cost reported in data-model |
| V. CCC-First Code Search | `ccc` before grep/read; refresh index after changes | PASS — plan derived from `ccc` searches; tasks will re-index |

**Hardware constraints**: VRAM writes V-Blank/H-Blank only (FR-008); OAM < 40 (FR-003); palette 0–7 (FR-009). No new banking introduced — modules link in the default bank.

**Initial gate**: PASS — no violations. Complexity Tracking not required.

## Project Structure

### Documentation (this feature)

```text
specs/001-visual-content-layer/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (header-level API contracts)
│   ├── sprite_manager.md
│   ├── background_layer.md
│   ├── palette.md
│   └── effects.md
└── tasks.md             # Phase 2 output (/speckit-tasks)
```

### Source Code (repository root)

```text
include/
├── sprite.h         # REWRITE: budget-aware sprite manager + VRAM tile ref-count API
├── background.h     # REWRITE: BG layer disjoint from UI/text; scroll hooks
├── palette.h        # NEW: runtime CGB palette-set assignment + DMG fallback
└── effects.h        # NEW: blink/fade/scroll/animation over a fixed EffectState array

src/
├── sprite.c         # REWRITE + RE-ENABLE in CMake: OAM alloc, tile ref-count reuse
├── background.c     # REWRITE + RE-ENABLE in CMake: BG tile-range mgmt, scroll
├── palette.c        # NEW: set_*_palette wrappers, palette-slot tracking, DMG no-op
├── effects.c        # NEW: per-VBL effect tick, fixed-size state array
├── utils.c          # REUSE: fade_to_black/from_black (fade effect may delegate)
└── game_system.c    # TOUCH: call sprite/bg/effect init; keep one-time sprite_data load

CMakeLists.txt       # EDIT: add src/sprite.c, src/background.c, src/palette.c, src/effects.c
```

**Structure Decision**: Single embedded-ROM project with the existing flat
`src/`+`include/` layout. The dead `sprite.c`/`background.c` (currently commented
out in `CMakeLists.txt`, lines 36–37) are revived and rewritten to satisfy the
budget constraints rather than introducing parallel modules — this honors
Principle IV's "reuse before allocate" and avoids name collisions with the
existing `Sprite`/`background_t` types. `palette.c` and `effects.c` are net-new
single-module files in the default bank.

## Complexity Tracking

> No Constitution Check violations — section intentionally empty.
