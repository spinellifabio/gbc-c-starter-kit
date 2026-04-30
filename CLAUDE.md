# GBC C Starter Kit

## Build

```bash
cmake --preset ninja-gbdk          # configure
cmake --build --preset build-rom   # build → rom/gbc-c-starter-kit.gbc
cmake --build build --target run   # launch in BGB
```

No automated tests. Verify manually in BGB or SameBoy.

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

`sprite.c` / `background.c` excluded from build (dead code, saves ~400 B WRAM).

VRAM sprites: 0–19 Alex idle, 20–99 Alex run, 100–103 treasure, 104–107 hazard.

CGB detected at runtime: `_cpu == CGB_TYPE`. DMG skips palettes gracefully.

## Code Style

- C99, no heap, no `<stdbool.h>`. `uint8_t` as bool.
- `uint8_t`/`uint16_t` preferred; explicit narrowing casts: `(uint8_t)(x + y)`.
- Hardware registers use unsigned literals: `0u`, `0xFFu`.
- 4-space indent, brace on same line, `static` all private symbols, ROM data `const`.

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
