# Contract: Palette Module (`include/palette.h`)

Runtime CGB palette-set assignment with DMG fallback. Follows
`examples/colorbar` and `examples/dscan`. Guards every color write with
`_cpu == CGB_TYPE`.

| Function | Signature | Contract |
|----------|-----------|----------|
| `palette_init` | `void palette_init(void)` | Clears BG/OBJ slot-usage bitmaps. |
| `palette_set_bkg` | `uint8_t palette_set_bkg(uint8_t slot, const palette_color_t* colors)` | On CGB: `set_bkg_palette(slot,1,colors)`, mark slot used, return slot. On DMG: no-op, return slot. `slot > 7` or all used → return `PALETTE_NONE` (0xFF) (FR-009/FR-011). |
| `palette_set_obj` | `uint8_t palette_set_obj(uint8_t slot, const palette_color_t* colors)` | As above for OBJ palettes. |
| `palette_alloc_bkg` | `uint8_t palette_alloc_bkg(const palette_color_t* colors)` | Finds first free BG slot, assigns, returns it; `PALETTE_NONE` if all 8 used. |
| `palette_free` | `void palette_free(uint8_t kind, uint8_t slot)` | Marks slot free for reuse. |
| `palette_dmg_grayscale` | `void palette_dmg_grayscale(void)` | Sets `BGP/OBP0/OBP1` to sensible grayscale (DMG path, FR-010). |

**Invariants**: slot 0–7; no silent overwrite of an in-use slot — over-limit
returns `PALETTE_NONE`. **DMG**: all color paths skipped gracefully; content
legible in grayscale.
