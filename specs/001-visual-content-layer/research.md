# Phase 0 Research: Visual Content Layer

All Technical Context items were resolvable from the existing codebase and the
`examples/` samples (constitution Principles IV & V). No open NEEDS
CLARIFICATION markers remain.

## R1 — VRAM tile reuse strategy (FR-002, FR-004, FR-005)

- **Decision**: Track a fixed table of tile-range allocation records, each
  `{ first_tile, tile_count, graphic_id, ref_count }`. On sprite alloc, look up
  `graphic_id`; if resident, increment `ref_count` and reuse `first_tile`; else
  find a free gap in the 88-tile budget, `set_sprite_data` once, and record it.
  On free, decrement `ref_count`; release the range only at zero.
- **Rationale**: Reference counting is the minimum mechanism that satisfies
  "duplicate graphics consume tiles once" (SC-002) and "release tiles no longer
  referenced" (FR-005) with O(1) WRAM per distinct graphic. Mirrors the existing
  one-time `set_sprite_data` load in `game_system.c` (loads sprite data once, not
  per reset — `OPTIMIZATION_REPORT.md` "Sprite VRAM Layout Fix").
- **Alternatives considered**: (a) No reuse / load-per-sprite — rejected, blows
  the 88-tile budget and re-introduces the overlap bug fixed earlier. (b) Free
  list of individual tiles — rejected, fragmentation handling costs more WRAM
  than the kit needs; whole-range records match how GBDK assets are emitted.

## R2 — OAM slot allocation within the sub-40 range (FR-003)

- **Decision**: Reserve the existing live ranges (player + `NPC_OAM_BASE` block
  from `npc.c`) and allocate sprite-manager handles from a free bitmap/cursor
  over the remaining OAM indices < 40. Use GBDK shadow OAM (`move_sprite`,
  `hide_sprites_range`) — never write hardware OAM directly.
- **Rationale**: `utils.c::scene_init_clean` documents that direct 0xFE00 writes
  are clobbered by the V-Blank DMA; the shadow buffer is the supported path.
  `npc.c` already partitions OAM via `NPC_OAM_BASE`; the manager must respect
  those reservations to avoid double-allocation.
- **Alternatives considered**: Full 0–39 dynamic pool ignoring existing
  reservations — rejected, would collide with player/NPC sprites.

## R3 — Background layer separation from UI/text (FR-006, FR-007)

- **Decision**: Keep one hardware BG map. Partition the BG tile-index space so
  background-content tiles occupy a range disjoint from the font tiles
  (`FONT_OFFSET`) and UI tiles already loaded in `game_system_init`. Text keeps
  using `print_centered` / dialogue helpers untouched.
- **Rationale**: The kit uses a single BG map plus the window for dialogue
  (`title_screen.c` uses `WY_REG`/`HIDE_WIN`, `set_bkg_tiles`). A second hardware
  map does not exist on the GB; "separate layer" = disciplined tile-range +
  palette partition (confirmed in spec Assumptions).
- **Alternatives considered**: Use the window layer as the "background" — rejected,
  the window is already owned by the dialogue box (UX Principle III).

## R4 — CGB palette assignment + DMG fallback (FR-009, FR-010, FR-011)

- **Decision**: Wrap `set_bkg_palette` / `set_sprite_palette` behind a palette
  module that tracks which of the 8 BG and 8 OBJ palette slots are in use. Guard
  all color writes with `_cpu == CGB_TYPE`; on DMG, set the DMG palette registers
  (`BGP_REG`/`OBP0_REG`/`OBP1_REG`) to sensible grayscale and no-op the color
  path. Report failure when a request exceeds 8 distinct slots.
- **Rationale**: Directly follows `examples/colorbar` (`set_bkg_palette(BKGF_CGB_PALn, …)`)
  and the existing `set_solid_bkg` / `game_system.c` `_cpu == CGB_TYPE` guard
  pattern (`utils.c:14`). DMG already handled via `0xE4` default palettes.
- **Alternatives considered**: Assume CGB always — rejected, violates Principle II
  (DMG graceful degradation) and FR-010.

## R5 — Effects engine with fixed WRAM state (FR-012–FR-018)

- **Decision**: One static `EffectState effects[MAX_EFFECTS]` array (small fixed
  `MAX_EFFECTS`, e.g. 4). Each slot holds `{ type, target_ref, period, counter,
  progress, mode, active }` in `uint8_t` fields. A single `effects_tick()` runs
  per V-Blank from the main loop, advancing only active slots. Blink toggles
  target visibility on a counter (reusing the `draw_blink_line` 30-frame cadence
  idiom from `title_screen.c`); fade delegates to `utils.c` `fade_to_black`/
  `fade_from_black` (DMG-safe LUT already there); scroll updates `SCX/SCY` per
  `examples/galaxy`; animation steps the sprite's tile via the sprite manager.
- **Rationale**: A single fixed array is the only structure permitted to grow
  WRAM (FR-016); `uint8_t` fields keep it tiny and Z80-cheap (Principle on
  performance). Reusing existing fade keeps ROM down. Detach-on-free (FR-017) is
  a target-validity check each tick.
- **Alternatives considered**: (a) Per-effect heap/callbacks — rejected, no heap
  + WRAM budget. (b) Effect ISR via STAT/LCD interrupt — deferred; main-loop
  V-Blank tick is simpler and sufficient; H-Blank reserved only for VRAM copy
  timing per `examples/hblank_copy`.

## R6 — VRAM-write timing (FR-008, SC-008)

- **Decision**: All `set_bkg_data` / `set_sprite_data` / map updates occur during
  V-Blank (`wait_vbl_done`) for bulk loads; scroll register writes are cheap and
  done in the V-Blank window. Reserve H-Blank copy (`examples/hblank_copy`) only
  if a future large mid-frame tile stream is needed — not required for this scope.
- **Rationale**: Matches existing kit practice (`game_system.c`, `title_screen.c`
  all update tiles around `wait_vbl_done`). Prevents Mode-3 corruption (no
  tearing → SC-008).
- **Alternatives considered**: Unconditional immediate writes — rejected, causes
  glitches during active rendering.

## R7 — Build integration (Principle IV)

- **Decision**: Re-enable `src/sprite.c` and `src/background.c` and add
  `src/palette.c`, `src/effects.c` to the `SRC_FILES` list in `CMakeLists.txt`
  (currently excluded at lines 36–37). Update the "unused — excluded" comment.
  All four are single-module, default-bank files (no `#pragma bank`).
- **Rationale**: Reviving the dead modules avoids duplicate `Sprite`/`background_t`
  symbols and honors "reuse before allocate". Single-module banking respects the
  multi-module-bank corruption rule (memory: `feedback_gbdk_banking`).
- **Alternatives considered**: New parallel modules (`sprite2.c`) — rejected,
  symbol/name duplication and confusion.
