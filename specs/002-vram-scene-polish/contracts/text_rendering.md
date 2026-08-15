# Contract: Font-Derived Text Rendering

Covers the font base-tile fix (R0) and the dialogue border fix (R1).
Touches `include/utils.h`, `src/utils.c`, `src/game_system.c`,
`src/title_screen.c`, `src/dialogue.c`.

## New/changed API

| Function | Signature | Contract |
|----------|-----------|----------|
| `text_set_font_base` | `void text_set_font_base(uint8_t first_tile)` | Called once from `game_system_init` with `((pmfont_handle)ibm_font)->first_tile`. Stores the base for all char→tile conversion. Mirrors the existing `dialogue_set_window_base_tile` call. |
| `text_font_base` | `uint8_t text_font_base(void)` | Returns the stored base. Default `0` before init. |
| `text_char_tile` | `uint8_t text_char_tile(char c)` | Returns `first_tile + (c - ' ')` for `c` in `' '..'~'`; returns the space tile for anything outside that range. **Must not clamp at `'Z'`.** |
| `clear_screen` | `void clear_screen(void)` (existing) | Now fills with `text_char_tile(' ')` instead of the literal `(uint8_t)' '`. Still one `fill_bkg_rect` per call plus `clear_attr_map()`. |

## Behavioural requirements

- **FR-001/FR-002**: `dialogue.c:char_to_tile` accepts the full printable ASCII
  range, so `'|'` (`0x7C`) resolves to a real glyph. `draw_dialogue_box` renders
  four corners, horizontal rules, and **visible vertical edges**. Lowercase
  up-casing may be retained, but the `> 'Z'` → space clamp MUST be removed.
- **FR-007**: `title_screen.c` MUST NOT pass a raw `char*` to `set_bkg_tiles`.
  It renders the logo graphic (see `contracts/scene_visuals.md`); any remaining
  text goes through `printf`/`gotoxy`/`print_centered`, which already map
  through the font correctly.
- **FR-005**: after the `clear_screen` change, `scene_init_clean()` leaves the
  BG filled with the font's space glyph, not `'@'`.

## Invariants

- No call site may hardcode `32` (or `' '`) as a BG tile index.
- Existing text helpers (`print_centered`, `cls`, `printf`, dialogue
  typewriter) remain the only text paths — Constitution Principle III.
- Behaviour is identical on DMG and CGB; the base tile is hardware-independent.

## WRAM cost

+1 byte (the cached base tile). No other growth.
