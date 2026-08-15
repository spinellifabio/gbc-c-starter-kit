# Phase 0 Research: VRAM & Scene Visual Polish

All findings below were derived via `ccc` semantic search followed by targeted
file reads (Constitution Principle V), then cross-checked against the BGB
screenshots in `docs/screenshots/`.

## R0 — Root cause: font base-tile assumption (explains most "compromised" scenes)

- **Decision**: Treat `font_ibm`'s actual `first_tile` as the single source of
  truth for any BG tile index derived from a character. Store it at init
  (alongside the existing `dialogue_set_window_base_tile`) and expose it to
  `utils.h` so `clear_screen()` and any direct `set_bkg_tiles` caller derives
  `tile = first_tile + (c - ' ')` instead of hardcoding the ASCII value.
- **Evidence**: `game_system.c:44-46` does
  `font_load(font_ibm)` → `dialogue_set_window_base_tile(((pmfont_handle)ibm_font)->first_tile)`.
  GBDK's `font_ibm` is **96 tiles** (`/c/gbdk/examples/cross-platform/fonts/src/fonts.c`
  comments it as "96 tiles"), covering ASCII `0x20..0x7F`, and loads at
  `first_tile = 0`.
  Two call sites assume `first_tile == 32` (ASCII == tile index):
  1. `include/utils.h:57` — `fill_bkg_rect(0,0,20,18,(uint8_t)' ')` fills the
     BG with tile **32**, which is font glyph 32 = ASCII `0x20+32` = **`'@'`**.
  2. `src/title_screen.c:36-37` — `set_bkg_tiles(..., (uint8_t*)g_settings.game_name)`
     writes raw ASCII bytes as tile indices.
  Arithmetic reproduces the screenshots exactly. For `game_name = "GBC Prototype"`
  (`game_settings.c:9`): `'G'`=71→glyph 71→`'g'`, `'B'`=66→`'b'`, `'C'`=67→`'c'`,
  `' '`=32→`'@'`, `'P'`=80→`'p'` ⇒ renders **`gbc@p`**, then `'r'`=114 exceeds the
  96-glyph font and pulls garbage — matching `Screenshot ...121923.png`
  (`gbc@p` / `PNRNP`). The `clear_screen()` case matches the full-screen `@`
  field in `Screenshot ...121646.png` (win screen).
- **Why this is the big one**: `scene_init_clean()` (`utils.c:26-57`) calls
  `clear_screen()`. Win screen and game over use `scene_init_clean()`; splash,
  title, options, and credits use GBDK's `cls()`, which maps through the font
  correctly. That is precisely why the user reports scenes are compromised
  "soprattutto dalla fase di gioco in poi".
- **Alternatives considered**: Reload the font at `first_tile = 32` so ASCII ==
  tile index — rejected: `font_load` chooses the base, it is not a caller
  parameter, and hardcoding a second assumption keeps the same fragility.

## R1 — Dialogue box border ("l'apposito bordo fornito dal font")

- **Decision**: Remove the `> 'Z'` clamp in `dialogue.c:char_to_tile` so the
  full printable ASCII range `' '..'~'` maps through, then draw the frame with
  the box-capable glyphs the font already provides (`|` for the vertical
  edges). Zero new VRAM, zero new art.
- **Evidence**: `dialogue.c:28-36` — `if ((uint8_t)c < ' ' || (uint8_t)c > 'Z') c = ' ';`.
  `'|'` is `0x7C` > `'Z'` (`0x5A`), so **every vertical border character is
  silently converted to a space** and the box has no left/right edges. This is
  visible in `Screenshot ...121012.png`: top and bottom rules (`+`/`-`) render,
  the side edges are blank. The clamp also destroys lowercase-adjacent glyphs
  above `'Z'`.
- **Note**: `set_win_char` uses `set_win_based_tiles(..., window_tile_base)`
  with `window_tile_base = first_tile`, so `first_tile + (c - ' ')` is already
  correct here — only the clamp is wrong. The dialogue path is therefore
  base-tile-agnostic and needs no R0 change.
- **Alternatives considered**: Author 6–9 dedicated border tiles into the free
  BG tile range — deferred; the font's own glyphs satisfy the requirement at no
  cost, and this matches the user's wording ("the border provided by the font").

## R2 — Free BG tile budget for the title logo

- **Decision**: Place the new title logo graphic in the free BG tile range.
- **Evidence**: BG tile map after init — font glyphs occupy **0–95**
  (96 tiles at `first_tile = 0`); `tileset` occupies **128–131**
  (`game_system.c:54`, `FONT_OFFSET = 128` from `world_defs.h:4`,
  `tileset_tiles[64]` = 4 tiles). Free: **96–127** (32 tiles) and **132+**.
  A logo fits with no displacement of font or tileset data, satisfying FR-011.
- **Alternatives considered**: Reuse the `FONT_OFFSET` region — rejected, it is
  already owned by the world tileset used by `gameplay.c:ensure_tile_quads`.

## R3 — Treasure / hazard sprite corruption (buffer over-read)

- **Decision**: Fix the tile-count/data-size mismatch in the object sprite
  uploads.
- **Evidence**: `game_system.c:75-76` calls
  `set_sprite_data(TREASURE_TILE, 4u, treasure_tiles)` and
  `set_sprite_data(HAZARD_TILE, 4u, hazard_tiles)`. `set_sprite_data` copies
  `count * 16` bytes, so it reads **64 bytes**, but `treasure_tiles[]` and
  `hazard_tiles[]` (`game_objects.c:9-22`) are each only **32 bytes**
  (4 rows × 8 bytes — 8×8 tile format, not the 16-byte 8×16 format).
  Each upload therefore over-reads 32 bytes past the end of the array,
  corrupting both object graphics. This matches the red/pink and blue garbage
  blocks in `Screenshot ...121622.png`.
- **Alternatives considered**: Reduce the count to 2 — insufficient on its own,
  because the sprite mode (R4) also expects 8×16 data.

## R4 — 8×16 sprite mode vs. 2×2 8×8 draw layout

- **Decision**: Align the actor draw routines with the hardware sprite mode.
- **Evidence**: `game_system.c:97` sets `SPRITES_8x16`. In 8×16 mode each OAM
  entry is 8 px wide and **16 px tall** and consumes a tile pair. But
  `gameplay.c:429-441` draws the player as a 2×2 grid of four OAM entries at
  `(x,y)`, `(x+8,y)`, `(x,y+8)`, `(x+8,y+8)`, and `npc.c:113-127` does the same.
  With 8×16 sprites the lower pair overlaps the upper pair's bottom half,
  producing the stretched/smeared actor in `Screenshot ...121622.png`.
  `game_objects.c:82-83` uses two side-by-side entries, which is correct for
  8×16 (16×16 result) once its tile data is fixed per R3.
- **Alternatives considered**: Switch globally to `SPRITES_8x8` — rejected, all
  `png2asset` metasprite assets declare `TILE_H 16` (`player_idle.h` etc.) and
  `move_metasprite_ex` in `intro.c` depends on 8×16.

## R5 — Sprite palette collisions

- **Decision**: Give each actor class its own OBJ palette slot and load the
  corresponding palette data; route assignment through the existing
  `palette.c` allocator rather than raw indices.
- **Evidence**: Only two OBJ palettes are ever loaded —
  `game_system.c:83-84` loads `player_idle_palettes` into slot **0** and
  `player_run_palettes` into slot **1**. `game_objects.c:86-89` then sets
  `set_sprite_prop(..., objects[i].palette - 1)`, so treasure (`palette = 1`)
  lands on slot **0** (the player's palette) and hazard (`palette = 2`) on slot
  **1** (the player-run palette) — direct palette bleed (FR-004). Palettes for
  `cat_idle`, `cutscene_player`, and `cutscene_npc` are never uploaded at all.
  `palette.c` already provides `palette_set_obj`/`palette_alloc_obj` with
  slot tracking and a DMG no-op, and is unused by these call sites.
- **Alternatives considered**: Hardcode more `set_sprite_palette` calls —
  rejected, it re-creates the untracked-slot problem the palette module exists
  to solve (Constitution Principle IV: reuse before allocating).

## R6 — Options screen row overflow

- **Decision**: Reduce the row origin and clip/pad each row to the remaining
  screen width instead of emitting fixed-length trailing padding.
- **Evidence**: `options_screen.c:76-79` starts at `gotoxy(2u, ...)` then emits
  `">"` (1) + `" DIFFICULTY: "` (13) + `"NORMAL"` (6) + `"      "` (6 pad)
  = column 2 + 26 = **28 columns** on a 20-column screen, so the tail wraps to
  the next line (`Screenshot ...121201.png` shows `NORM` / `AL`). The build
  line `options_screen.c:115-116` at `gotoxy(2u,3u)` prints
  `"GBC Prototype v0.2.0"` (20 chars) from column 2 = **22 columns**, also
  wrapping (`.GBC Prototype v0.2` / `.0`).
- **Alternatives considered**: Shorten only the localized label strings —
  rejected, it leaves the defect latent for any future or longer translation
  (spec Edge Cases).

## R7 — Cutscene actors are the player sprite

- **Decision**: Point the intro cutscene at the dedicated cutscene tile bases
  and give the two actors distinct palettes.
- **Evidence**: `intro.c:92-105` draws **both** the lead and the buddy from
  `player_run_metasprites` / `player_idle_metasprites` with `base_prop = 0`
  for both, so two identical player sprites appear (`Screenshot ...121012.png`).
  The reserved bases `CUTSCENE_PLAYER_TILE = 128` and `CUTSCENE_NPC_TILE = 148`
  (`game_objects.h:42-43`) are loaded but with `player_idle_tiles` for both
  (`game_system.c:79-80`, comment: "share player_idle tile data until final art
  is added"). Per the clarification session, distinct art is in scope (FR-012).
- **Note**: This is sprite VRAM, indices 0–255, disjoint from the BG tile
  numbering in R0/R2 — no interaction between the two budgets.

## R8 — Splash screen leftover sprite

- **Decision**: Hide the shadow OAM range before drawing the splash screens.
- **Evidence**: `splash.c:12-26` sets the BG and prints text but never hides
  sprites; `scene_init_clean()` (which does `hide_sprites_range(0,40)`) is not
  called. Stale OAM entries from a prior scene remain visible — the beige block
  at the top-left of `Screenshot ...121121.png`.
- **Alternatives considered**: Call full `scene_init_clean()` — rejected until
  R0 lands, because it currently routes through the broken `clear_screen()`.

## R9 — Bank-1 overflow discovered during implementation (post-hoc)

- **Symptom**: after T001–T023, the ROM built clean but hung with a white
  screen from frame 0 in BGB. A full-memory dump showed the cartridge header
  region (fixed bytes at file offset 0x100) reading as uniform `0xFC` instead
  of the real Nintendo logo bytes — BGB's fill pattern for an out-of-range
  MBC5 ROM-bank select, i.e. the ROM bank register had been corrupted by
  garbage execution.
- **Root cause**: `ihxcheck` warned "Write from one bank spans into the next"
  at the linker stage — informational-looking, but real. The linker map
  showed `_CODE` (30070 B) + `__HOME` (2452 B) = 32522 B against a 32256 B
  budget (bank0+bank1, 0x0200–0x7FFF), a **266-byte overflow** spilling past
  0x8000 (VRAM at runtime, not addressable ROM). SDCC's `GSINIT` — the
  routine that initializes every global variable before `main()` runs — was
  itself linked at 0x8067, inside the invalid region. This predates this
  feature (the original baseline build already had a smaller-but-similar
  overflow landing on inert bytes); this feature's net code growth pushed a
  different, load-bearing chunk into the danger zone.
- **Two banking fixes were attempted and reverted** — manually `#pragma bank`ing
  `res/tiles/logo.c` first with a hardcoded bank number, then with GBDK-2020's
  `-autobank`/`#pragma bank 255` auto-assignment plus `-Wl-j -autobank
  -Wb-ext=.rel` in `CMakeLists.txt`. **Both made the overflow worse**
  (introduced explicit "Multiple write... Possible overflow from Bank 0 into
  Bank N" linker collisions not present before), confirming a lack of
  reliable command of this toolchain's bank-packing behavior in this
  environment. Both were fully reverted; no banking infrastructure remains.
- **Actual fix**: `include/utils.h` declared 5 small `static inline` helpers
  (`wait_any_key`, `clear_attr_map`, `clear_screen`, `int_to_str`,
  `print_centered`) used by nearly every `.c` file in the project. SDCC does
  not reliably fold `static inline` — it was compiling a full copy of each
  into every one of the ~20 including translation units. Moving all five into
  single real definitions in `src/utils.c` (prototypes only in the header)
  shrank `_CODE` from 30070 to 24608 bytes — a 5462-byte reduction — closing
  the 266-byte overflow with **~5.2 KB of headroom to spare**. Verified by a
  full clean rebuild producing zero linker warnings (not just a smaller one).
- **Bonus find while touching `clear_attr_map`**: its old `#ifdef CGB` branch
  was permanently dead — `CGB` is never defined anywhere in the build
  (`cmake/gbdk-toolchain.cmake` only sets `-O2`). This means CGB attribute-map
  clearing between scenes has been a silent no-op since it was written,
  undermining FR-010 project-wide, not just for this feature's scenes.
  Rewritten to the project's actual convention, runtime `_cpu == CGB_TYPE`
  detection (matching `game_system.c`).

## Open items to confirm on hardware/emulator

- The exact `first_tile` value is asserted as `0` from screenshot arithmetic;
  the fix in R0 derives it at runtime and is therefore correct for any value.
  Confirm the observed value once in the BGB VRAM viewer during validation.
- The beige banding on the credits screen (`Screenshot ...121659.png`) is not
  yet root-caused; credits uses `cls()` and is expected to be clean after R0.
  Re-check it after R0/R5 land before opening a separate defect (FR-006).
