---
name: gbc-assets
description: Run the asset pipeline - turn PNG sprites/tilesets (and later Tiled maps) dropped in assets/ into res/ C sources via tools/gen_assets.py, check the VRAM budget, and wire them into scenes. Use when the user adds or changes art in assets/, or asks to import a sprite/tileset/map.
---

# GBC asset pipeline

```
assets/*.png  +  assets/assets.json  --python tools/gen_assets.py-->  res/sprites|tiles/*.c/.h
```

Generated files are committed. Build does not need Python; only regenerating does.

## Workflow

1. Look at what the user dropped: `ls assets/*`; read PNG sizes (Read the image, or `file <png>`).
2. Add entries to `assets/assets.json` (schema below). Name = C symbol prefix, `snake_case`.
3. `python tools/gen_assets.py [name ...]` (or `cmake --build build --target assets`).
   Rejects PNGs that are not a multiple of `frame`. Prints per-sprite tile cost (legacy
   included) and `sprite VRAM n/256`; exits non-zero on overflow.
4. `cmake --build --preset build-rom`, verify in BGB/SameBoy.
5. Report RAM/VRAM/ROM cost of what was added (project treats memory as a budget).

Before overwriting an existing entry's output, `git diff` the result: regeneration of
the committed player/tileset files differs only by header guard + whitespace.

## Manifest (`assets/assets.json`, paths relative to `assets/`)

```json
"sprites": {
  "<name>": {
    "src": "sprites/x.png",
    "frame": [16, 32],        // metasprite frame size w,h (-sw -sh)
    "pivot": [8, 32],         // optional (-px -py)
    "hitbox": [16, 32],       // optional (-pw -ph)
    "flags": []               // optional extra png2asset flags
  }
},
"tilesets": {
  "<name>": { "src": "tileset/x.png", "flags": ["-map", "-use_map_attributes", "-noflip"] }
}
```

Frames are indexed into `<name>_metasprites[]` (from `res/sprites/<name>.h`).

## Constraints (do not break)

- Sprites: 8x16 OAM mode, <= 4 CGB palettes per sprite, 256 sprite tiles total in VRAM.
  The budget also counts legacy sprites (`cat_idle`, `cutscene_*`) and the `*_tiles[N]` arrays
  in `src/game_objects.c` (treasure/hazard).
- The generator does NOT place tiles in VRAM. After adding a sprite, give it the next free slot
  (= budget total before adding) by hand: `*_TILE` define in `include/game_objects.h`,
  `set_sprite_data(...)` + layout comment in `src/game_system.c`, palette slot in the CGB block.
  Palettes: 8 total, slots 0-2 already used (player_idle, player_run) - check the comment there.
- Banking: never use multi-module `#pragma bank N` (corrupts ROM). Keep assets in the fixed bank.
- New `.c` under `res/<dir>/` is picked up by CMake glob (`res/*/*.c`); re-run configure for new files.
- Code style: see `CLAUDE.md` (C99, `uint8_t`, `static` privates, `g_` globals).

## Not built yet (do when real material arrives)

- **Tiled maps** (`.tmx`/`.json`): add a `"maps"` section to the manifest and extend
  `gen_assets.py` (stdlib `json` / `xml.etree`) to emit tilemap + collision LUT (`const uint8_t`)
  + entity table (spawn/NPC/trigger) from layers/object layers. PNG-only maps use `png2asset -map`
  through the `tilesets` section already.
- **Animation data** (named frame ranges per sprite, e.g. `"anims": {"run": [0, 24]}` in the
  manifest -> `res/generated/assets_gen.h` with `_ANIM_<NAME>_START/_LEN`, plus `-I res/generated`
  in CMake). Removed as unused; add together with the first consumer (the `Scene` work below).
- **Data-driven `Scene`** (struct with map, tileset, entities, palette) replacing hardcoded
  setup in `src/gameplay.c`. Migrate one screen at a time, `gameplay.c` last; use `examples/` samples
  as the GBDK reference. Ask the user before starting.
