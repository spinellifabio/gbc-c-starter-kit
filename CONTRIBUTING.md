# GBC C Starter Kit

Game Boy Color starter kit in C on gbdk-2020. Status and structure: `README.md`.

## Read first
- `.specify/memory/constitution.md` for project principles.
- `docs/Roadmap.md` for planned work; items there are not requirements until built.
- `specs/` for per-feature specs and plans (latest: `specs/002-vram-scene-polish/plan.md`).

## Priorities
CPU cycles first (target 60 FPS) → RAM/ROM → battery → maintainability.
Treat WRAM/VRAM/ROM as a budget and report the RAM cost of each feature.

## Commands
```bash
cmake --preset ninja-gbdk          # configure
cmake --build --preset build-rom   # build → rom/gbc-c-starter-kit.gbc
cmake --build build --target run   # launch in BGB
```
Tests: `python -m unittest discover -s tests` (asset pipeline + `save.c` run headless in BGB).
Everything else: verify manually in BGB or SameBoy.

## Architecture
Screen flow: splash → intro → title → [options] → gameplay → game over/win → credits → title

| File | Purpose |
|------|---------|
| `src/main.c` | Top-level screen flow |
| `src/game_system.c` | HW init: display, VRAM, font, palettes, sprites |
| `src/game_settings.c` | `g_settings` — persists across screens |
| `src/game_state.c` | Per-run state: lives, score, flags |
| `src/gameplay.c` | Game loop, movement, collision, camera |
| `src/dialogue.c` | Sliding dialogue box, typewriter effect |
| `src/input.c` | `get_pressed()` + `flush_input()` |
| `src/utils.c` | `fade_to_black()`, `clear_screen()`, `print_centered()`, `int_to_str()` |
| `include/world_defs.h` | `MAP_WIDTH`, `TILE_SIZE`, `FONT_OFFSET` |

`sprite.c` / `background.c` are excluded from the build (dead code, saves ~400 B WRAM).

VRAM sprites: 0–19 Alex idle, 20–99 Alex run, 100–103 treasure, 104–107 hazard.

CGB detected at runtime: `_cpu == CGB_TYPE`. DMG skips palettes gracefully.

## Examples
`examples/` is the canonical reference for GBDK techniques: follow the matching sample, and never
edit it to fit the game.

| Project | Purpose |
|---------|---------|
| `template_minimal` | Minimal game loop, scaffold for a new project |
| `colorbar` | CGB palettes, dual-bank VRAM tile/attribute split |
| `hblank_copy` | HBlank ISR, fast VRAM animation with dual banking |
| `sound` | All 4 audio channels, direct register control |
| `space` | Sprites, scrolling, input, complete mini-game |

## Code style
- C99, no heap, no `<stdbool.h>`. `uint8_t` as bool.
- `uint8_t`/`uint16_t` preferred; explicit narrowing casts: `(uint8_t)(x + y)`.
- Hardware registers use unsigned literals: `0u`, `0xFFu`.
- 4-space indent, brace on same line, `static` all private symbols, ROM data `const`.
- Do not use multi-module `#pragma bank N`: it corrupts the ROM in GBDK. Single-module banks only.

### Naming
| Kind | Convention | Example |
|------|-----------|---------|
| Functions | `snake_case` | `sprite_alloc` |
| Types/structs/enums | `PascalCase` | `GameSettings` |
| Enum values / macros | `UPPER_SNAKE_CASE` | `MAX_SPRITES` |
| Globals | `g_` prefix | `g_settings` |
| Header guards | `FILENAME_H` | `#ifndef SPRITE_H` |

### Performance
- `uint8_t` loop counters (Z80 cycles).
- LUTs in ROM over runtime div/mod.
- VRAM writes outside LCD Mode 3 (`wait_vbl_done()` or H-Blank ISR).
- OAM index < 40; palette 0–7.

### Include order (`.c` files)
1. Own `.h`
2. GBDK system (`<gb/gb.h>`, `<gb/cgb.h>`, `<stdint.h>`)
3. Project headers
4. Resource headers last

Use `#ifndef` guards, not `#pragma once`.

## Workflow
- Git Flow: `feature/*` from `develop`, releases from `release/*`, tags on `master`.
- Commits: Conventional Commits (`fix`, `feat`, `perf`, `refactor`, `docs`, `chore`); ask before committing.
- Changelog is generated with git-cliff (`cliff.toml`).
