# Phase 1 Data Model: Visual Content Layer

All structures are fixed-size, statically allocated (no heap). Field types are
`uint8_t`/`uint16_t` per Code Style. WRAM cost is reported per Principle IV.

## Entity: Sprite handle (`Sprite`)

Rewrite of the existing `include/sprite.h` struct; adds a graphic reference for
tile reuse.

| Field | Type | Meaning |
|-------|------|---------|
| `id` | `uint8_t` | Assigned OAM index (< 40) |
| `flags` | `uint8_t` | `SPRITE_FLAG_ACTIVE` / `SPRITE_FLAG_VISIBLE` |
| `graphic_id` | `uint8_t` | Index into the tile-allocation table (links tile reuse) |
| `tile_id` | `uint8_t` | First VRAM tile (cached from the allocation record) |
| `x`, `y` | `uint8_t` | Screen position |
| `width`, `height` | `uint8_t` | Tile dimensions (1–2) |
| `palette` | `uint8_t` | OBJ palette slot (0–7) |
| `priority`, `flip_x`, `flip_y` | `uint8_t` | OAM property bits |

- **Validation**: `id < 40`; `palette ≤ 7`; ops are no-ops unless `SPRITE_FLAG_ACTIVE`.
- **Lifecycle**: free → `sprite_alloc()` (active) → updates → `sprite_free()` (free, ref-count decremented).
- **WRAM**: `MAX_SPRITES` (32) × ~11 B ≈ 352 B (same as the previously-excluded module's footprint — net-justified by the feature).

## Entity: VRAM tile allocation record (`TileAlloc`)

New table backing FR-002/FR-004/FR-005.

| Field | Type | Meaning |
|-------|------|---------|
| `graphic_id` | `uint8_t` | Caller-facing graphic identity (key) |
| `first_tile` | `uint8_t` | First VRAM sprite-tile index |
| `tile_count` | `uint8_t` | Tiles occupied |
| `ref_count` | `uint8_t` | Sprites currently referencing it |

- **Validation**: `first_tile + tile_count ≤ 256`; running sum of in-use tiles must keep free ≥ 0 within the 88-tile budget; alloc returns failure when no gap fits.
- **State transitions**: free → resident (`ref_count` 1 on first request) → shared (`ref_count`++) → released (`ref_count` 0).
- **WRAM**: `MAX_TILE_ALLOCS` (≈16) × 4 B ≈ 64 B.

## Entity: Palette set (`palette.h`)

Tracks hardware palette-slot occupancy; not a heavy struct — a pair of small
usage bitmaps plus the assignment call.

| State | Type | Meaning |
|-------|------|---------|
| `bkg_slots_used` | `uint8_t` (bitmask) | Which of BG palettes 0–7 are assigned |
| `obj_slots_used` | `uint8_t` (bitmask) | Which of OBJ palettes 0–7 are assigned |

- **Validation**: slot index 0–7; assignment fails (returns sentinel) when all 8 of the requested kind are used (FR-011).
- **DMG behavior**: color assignment is a no-op; DMG registers set to grayscale (FR-010).
- **WRAM**: 2 B.

## Entity: Effect state (`EffectState`)

The single fixed structure permitted to grow WRAM (FR-016). Stored as
`static EffectState effects[MAX_EFFECTS];`.

| Field | Type | Meaning |
|-------|------|---------|
| `type` | `uint8_t` | `EFFECT_NONE/BLINK/FADE/SCROLL/ANIM` |
| `target` | `uint8_t` | Target ref (sprite id / background id) |
| `period` | `uint8_t` | Frames per step/toggle (clamped ≥ 1, FR-018) |
| `counter` | `uint8_t` | Frames since last step |
| `progress` | `uint8_t` | Current step / frame / fade level |
| `mode` | `uint8_t` | `LOOP` / `ONCE` (animation), direction (fade) |
| `active` | `uint8_t` | 0 = free slot, 1 = running |

- **Validation**: `period == 0` clamped to 1 (no divide-by-zero / infinite loop); a target that is freed clears the slot on next tick (FR-017).
- **Lifecycle**: free → `effect_start_*()` (active) → `effects_tick()` advances `counter`/`progress` → completes (`ONCE`) or loops → freed.
- **WRAM**: `MAX_EFFECTS` (4) × 7 B = 28 B — the only effects-API allocation.

## Relationships

```text
Sprite.graphic_id ──▶ TileAlloc (ref-counted; many sprites → one record)
Sprite.palette    ──▶ palette OBJ slot (0–7)
background_t.palette ─▶ palette BG slot (0–7)
EffectState.target ──▶ Sprite.id  (BLINK / ANIM)
EffectState.target ──▶ background id (SCROLL)
EffectState (FADE) ──▶ global palettes via utils.c fade helpers
```

## Aggregate WRAM budget (new/changed)

| Structure | Bytes | Note |
|-----------|-------|------|
| `Sprite[32]` | ~352 | Revived module (previously excluded) |
| `TileAlloc[16]` | ~64 | New, enables tile reuse |
| palette bitmaps | 2 | New |
| `EffectState[4]` | 28 | New — effects' only growth (FR-016) |
| **Total new** | **~446 B** | Reported per Principle IV; tune `MAX_*` if tighter budget needed |

> If WRAM pressure is found at build time, reduce `MAX_SPRITES`, `MAX_TILE_ALLOCS`,
> or `MAX_EFFECTS` — all are single `#define`s.
