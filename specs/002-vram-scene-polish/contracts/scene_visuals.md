# Contract: Scene Sprite, Palette & Layout Correctness

Covers R3–R8. Touches `src/game_system.c`, `src/game_objects.c`,
`src/gameplay.c`, `src/npc.c`, `src/intro.c`, `src/splash.c`,
`src/options_screen.c`, plus new art under `res/sprites/` and `res/tiles/`.

## Sprite data uploads (FR-003)

| Requirement | Contract |
|-------------|----------|
| Data size | Every `set_sprite_data(base, count, data)` MUST be backed by exactly `count * 16` bytes. `treasure_tiles[]` and `hazard_tiles[]` are re-authored to 8×16 format (64 bytes each for 4 tiles) or the count is corrected to match the real data. |
| No over-read | No upload may read past the end of its source array. |
| Ranges | Bases stay at the values in `game_objects.h` (`TREASURE_TILE` 100, `HAZARD_TILE` 104, `CAT_IDLE_TILE` 108, `CUTSCENE_PLAYER_TILE` 128, `CUTSCENE_NPC_TILE` 148). |

## Actor drawing in 8×16 mode (FR-003)

| Requirement | Contract |
|-------------|----------|
| OAM geometry | With `SPRITES_8x16`, a 16×16 actor uses **2** OAM entries at `(x, y)` and `(x+8, y)`. The 2×2 four-entry layout in `gameplay.c:429-441` and `npc.c:113-127` is replaced. |
| Tile pairing | Consecutive tile indices per 8×16 column; base index follows the 8×16 pairing rule. |
| OAM budget | Index < 40 always. Player moves from 4 entries (20–23) to 2 (20–21); freed slots stay unused. |

## Palette assignment (FR-004, FR-010)

| Requirement | Contract |
|-------------|----------|
| Distinct slots | Each actor class gets its own OBJ slot per `data-model.md` E4: player 0, player_run 1, treasure 2, hazard 3, cat 4, cutscene player 5, cutscene NPC 6. |
| Loaded data | Every slot in use MUST have had its palette uploaded. `cat_idle`, `cutscene_player`, `cutscene_npc` palettes are now loaded. |
| Routing | Assignment goes through `palette_set_obj` / `palette_alloc_obj` (`palette.c`) so `obj_slots_used` stays accurate. No raw `set_sprite_palette` at the call sites. |
| Prop writes | `set_sprite_prop` writes the intended slot for **every** active object, including when `palette == 0` (no stale prop carry-over). |
| DMG | All palette work guarded by `_cpu == CGB_TYPE`; DMG falls back to the `0xE4` defaults via `palette_dmg_grayscale`. |
| Transitions | Scene changes reset palette bookkeeping so no assignment leaks forward. |

## Distinct actor art (FR-012)

| Asset | Base | Contract |
|-------|------|----------|
| `cat_idle` | 108 | 20 tiles, visually distinct from `player_idle`. |
| `cutscene_player` | 128 | 20 tiles, distinct from the gameplay player. |
| `cutscene_npc` | 148 | 20 tiles, distinct from `cutscene_player`. |

`intro.c` MUST draw the lead from `CUTSCENE_PLAYER_TILE` and the buddy from
`CUTSCENE_NPC_TILE` with distinct `base_prop` palettes — not `player_run` /
`player_idle` with `base_prop = 0` for both.

## Title logo (FR-007)

| Requirement | Contract |
|-------------|----------|
| Placement | BG tiles 96–127 (≤ 32 tiles), disjoint from font (0–95) and tileset (128–131). |
| Rendering | Drawn via `set_bkg_tiles` with an explicit tile-index map — never a raw `char*`. |
| Palette | BG palette 0, or slot 2 if a second palette is needed. DMG-safe. |
| Timing | Uploaded during `DISPLAY_OFF` at init, or inside `wait_vbl_done()`. |

## Options screen layout (FR-008, FR-009)

| Requirement | Contract |
|-------------|----------|
| Width | Every row (cursor + label + value + padding) MUST fit within 20 columns from its origin column. |
| Padding | Trailing erase-padding is clipped to the remaining width, never emitted as a fixed-length run that overflows. |
| Build line | The `game_name` + `version` line fits in 20 columns (shorten the origin, the format, or both). |
| Palette | One consistent BG text palette across all rows on CGB. |
| Longer strings | A longer localized value truncates rather than wrapping (spec Edge Cases). |

## Splash screen (FR-010)

`splash.c` hides the shadow OAM range (`hide_sprites_range(0u, 40u)` followed
by a `wait_vbl_done()` so the DMA lands) before drawing, so no stale sprite
from a prior scene remains visible.

## Credits (FR-006)

`credits.c` is the non-regression baseline. It keeps its `cls()` +
`clear_attr_map()` path and MUST render at least as correctly after these
changes as before.

## VRAM/OAM timing (Constitution Hardware Constraints)

All VRAM and palette writes occur with the display off or inside
`wait_vbl_done()` / the H-Blank ISR — never during LCD Mode 3.
