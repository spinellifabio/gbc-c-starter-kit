# Contract: Sprite Manager (`include/sprite.h`)

Public API the sprite manager exposes. Rewrite of the existing header; adds
tile-reuse semantics. All functions are no-ops on an inactive/NULL handle.

| Function | Signature | Contract |
|----------|-----------|----------|
| `sprite_init` | `void sprite_init(void)` | Clears handle table + tile-allocation table; reserves live OAM ranges (player, `NPC_OAM_BASE`). Idempotent. |
| `sprite_alloc` | `Sprite* sprite_alloc(uint8_t graphic_id, const uint8_t* tiles, uint8_t tile_count)` | Returns a handle. If `graphic_id` already resident → reuse tiles (no `set_sprite_data`, ref_count++). Else load `tile_count` tiles into a free gap within the 88-tile budget. Returns `NULL` if no free OAM slot OR no tile gap (FR-003/FR-004). |
| `sprite_free` | `void sprite_free(Sprite*)` | Releases OAM slot; decrements graphic ref_count; frees tile range at zero (FR-005). Detaches active effects targeting it (FR-017). |
| `sprite_set_pos` | `void sprite_set_pos(Sprite*, uint8_t x, uint8_t y)` | Updates shadow OAM via `move_sprite`. |
| `sprite_set_visible` | `void sprite_set_visible(Sprite*, uint8_t)` | Shows/hides (hide = `move_sprite(id,0,0)`). |
| `sprite_set_tiles` | `void sprite_set_tiles(Sprite*, uint8_t tile_id)` | Repoints tile (animation); honors 2×2 layout, never exceeds id < 40. |
| `sprite_set_palette` | `void sprite_set_palette(Sprite*, uint8_t pal)` | `pal` 0–7. |
| `sprite_update_all` | `void sprite_update_all(void)` | Flushes active handles to shadow OAM. |
| `sprite_clear_all` | `void sprite_clear_all(void)` | Hides 0–39 via `hide_sprites_range` semantics. |

**Invariants**: never writes hardware OAM (0xFE00) directly; OAM index always
< 40; total resident tiles ≤ 88 free. **Errors**: surfaced as `NULL` return
(alloc) — never silent overwrite.
