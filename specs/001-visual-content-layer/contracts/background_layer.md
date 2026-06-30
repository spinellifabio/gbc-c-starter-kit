# Contract: Background Layer (`include/background.h`)

Rewrite of existing header. One hardware BG map; background-content tiles occupy
a range disjoint from font (`FONT_OFFSET`) and UI tiles. Does not touch the
window layer (owned by the dialogue box).

| Function | Signature | Contract |
|----------|-----------|----------|
| `bg_init` | `void bg_init(void)` | Resets layer state, scroll = 0. Does not clear font/UI tiles. |
| `bg_load` | `void bg_load(background_t*, const uint8_t* tiles, const uint8_t* map, const uint8_t* attrmap, uint8_t w, uint8_t h)` | Loads tiles into the BG content range (not over `FONT_OFFSET`). CGB attrmap via `VBK_REG` bank 1 only when `_cpu == CGB_TYPE` (FR-006/FR-007). Writes during V-Blank (FR-008). |
| `bg_set_scroll` | `void bg_set_scroll(background_t*, int8_t dx, int8_t dy)` | Adjusts `SCX/SCY` per `examples/galaxy`; no tearing (V-Blank window). |
| `bg_set_visible` | `void bg_set_visible(background_t*, uint8_t)` | Toggles BG content visibility without disturbing UI/text. |
| `bg_set_tile` | `void bg_set_tile(background_t*, uint8_t x, uint8_t y, uint8_t tile)` | Single map cell update (V-Blank). |
| `bg_set_palette` | `void bg_set_palette(background_t*, uint8_t pal)` | Delegates to palette module; DMG-safe. |
| `bg_clear` | `void bg_clear(background_t*)` | Clears only the BG content range; UI/text intact (FR-007). |

**Invariants**: tile-index range disjoint from font/UI; all VRAM/map writes
outside Mode 3; never renders into the dialogue window. **Coexistence test**:
dialogue typewriter must render correctly over an active background.
