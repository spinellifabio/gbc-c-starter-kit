# Phase 1 Data Model: VRAM & Scene Visual Polish

This feature introduces no new gameplay entities. Its "data model" is the set
of hardware resource maps (BG tiles, sprite tiles, OAM slots, palette slots)
that the fixes must keep consistent. Values below are the **target** state.

## E1 — BG tile map (256-entry index space)

| Range | Owner | Count | Status |
|-------|-------|-------|--------|
| 0–95 | `font_ibm` glyphs (ASCII `0x20`–`0x7F`) | 96 | Existing, `first_tile = 0` |
| 96–127 | **Title logo graphic** | ≤ 32 | **NEW** (FR-007) |
| 128–131 | `tileset` world tiles (`FONT_OFFSET`) | 4 | Existing |
| 132–255 | Free | 124 | Unused |

**Invariant**: any BG tile index derived from a character MUST be computed as
`font_first_tile + (c - ' ')`. No call site may assume `first_tile == 32`
(the ASCII value). Validation: `clear_screen()` and `title_screen.c` both go
through the derived value.

**Net cost**: title logo consumes previously-unused tiles 96–127. No font or
tileset data is displaced ⇒ FR-011 satisfied with zero displacement.

## E2 — Sprite tile map (256-entry index space, 8×16 mode)

| Range | Owner | Count | Change |
|-------|-------|-------|--------|
| 0–19 | `player_idle` | 20 | Unchanged |
| 20–99 | `player_run` | 80 | Unchanged |
| 100–103 | `obj_treasure` | 4 | **Data size fixed** (R3) |
| 104–107 | `obj_hazard` | 4 | **Data size fixed** (R3) |
| 108–127 | `cat_idle` | 20 | **Distinct art** (FR-012) |
| 128–147 | `cutscene_player` | 20 | **Distinct art** (FR-012) |
| 148–167 | `cutscene_npc` | 20 | **Distinct art** (FR-012) |
| 168–255 | Free pool (`SPRITE_FIRST_FREE_TILE`) | 88 | Unchanged |

**Invariant**: every `set_sprite_data(base, count, data)` MUST be backed by
exactly `count * 16` bytes of source data (8×16 mode). Current violation:
`treasure_tiles[]` / `hazard_tiles[]` supply 32 bytes for a declared count of 4
(needs 64) — a 32-byte over-read each.

**Net cost**: zero new tiles. Ranges are unchanged; only the backing data is
corrected and re-authored.

## E3 — OAM slot map (40 hardware entries)

| Range | Owner | Scene | Notes |
|-------|-------|-------|-------|
| 0–19 | Game objects (`i*2`, `i*2+1`, i = 0..9) | Gameplay | 2 entries/object |
| 20–23 | Player | Gameplay | Collision with objects already fixed |
| 0–3 | Cutscene lead | Intro | Scene-exclusive, no overlap with gameplay |
| 4–7 | Cutscene buddy | Intro | Scene-exclusive |
| 24–39 | Free | — | |

**Invariant**: OAM index < 40 always. Actors drawn in 8×16 mode occupy one OAM
entry per 8×16 column — a 16×16 actor uses **2** entries side-by-side, never a
2×2 grid of 4 (current violation in `gameplay.c:429-441` and `npc.c:113-127`).

**Net cost**: the player drops from 4 OAM entries to 2, freeing 2 slots.

## E4 — CGB palette slots (8 BG + 8 OBJ)

### OBJ palettes (target)

Corrected during implementation: `player_run_PALETTE_COUNT = 2`, so the
existing `set_sprite_palette(1u, 2u, player_run_palettes)` call occupies
**two** slots (1 and 2), not one. Object palettes start at slot 3.

| Slot | Owner | Status |
|------|-------|--------|
| 0 | `player_idle` | Existing |
| 1–2 | `player_run` (2 palettes) | Existing |
| 3 | `obj_treasure` | **NEW** — previously aliased slot 0 (`palette - 1` = 0) |
| 4 | `obj_hazard` | **NEW** — previously aliased slot 1 (`palette - 1` = 1) |
| 5 | `cat_idle` | **NEW** — never loaded today |
| 6 | `cutscene_player` | **NEW** — never loaded today |
| 7 | `cutscene_npc` | **NEW** — never loaded today |

All 8 OBJ slots are assigned after this feature — zero spare, still within
the 0–7 budget (FR-011).

### BG palettes

| Slot | Owner | Status |
|------|-------|--------|
| 0 | Per-scene solid colour (`set_solid_bkg`) + text | Existing |
| 1 | `tileset` attributes | Existing |
| 2 | Title logo | **NEW** (only if the logo needs a second palette) |
| 3–7 | Free | |

**Invariants**: slot index 0–7; assignment goes through `palette_set_obj` /
`palette_alloc_obj` (`palette.c`) so `obj_slots_used` tracking stays accurate;
all palette writes are guarded by `_cpu == CGB_TYPE` so DMG degrades to the
`0xE4` grayscale defaults.

**State transition**: on every screen transition, palette slot bookkeeping is
reset so no stale assignment leaks into the next scene (FR-010).

## E5 — Screen → clear-path map

Which scenes clear the BG through which helper. This is the axis that
determined which screens looked broken.

| Screen | Clear path | Today | After fix |
|--------|-----------|-------|-----------|
| Splash | `cls()` + `clear_attr_map()` | OK (BG); stale OAM | + hide sprites (R8) |
| Intro | `clear_intro_background()` + `cls()` | OK | Unchanged |
| Title | `cls()` + `clear_attr_map()` | OK (BG); text garbled | Logo + derived tiles |
| Options | `cls()` + `clear_attr_map()` | OK; rows overflow | Row layout clipped |
| Gameplay | own viewport render | OK | Sprite mode/palette fixes |
| Game over | `scene_init_clean()` → `clear_screen()` | **`@` field** | Derived space tile |
| Win | `scene_init_clean()` → `clear_screen()` | **`@` field** | Derived space tile |
| Credits | `cls()` + `clear_attr_map()` | Near-correct (baseline) | Preserved (FR-006) |

## WRAM cost summary (Constitution Principle IV)

| Item | Bytes |
|------|-------|
| Cached font `first_tile` (module-level `uint8_t`) | +1 |
| Everything else (sprite mode, palettes, layout, art) | 0 — ROM `const` / existing slots |
| **Net WRAM growth** | **+1 byte** |

New art (title logo, three actor graphics) lands in ROM as `const` data, not
RAM, and reuses already-reserved VRAM ranges.
