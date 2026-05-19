# GBDK Examples

Curated reference examples from [gbdk-2020](https://github.com/gbdk-2020/gbdk-2020). Each builds independently with its own Makefile.

## Building an example

Requires `GBDK_HOME` set to your GBDK installation:

```bat
set GBDK_HOME=C:/gbdk/
cd template_minimal
compile.bat
```

Or via Makefile on Linux/macOS:

```bash
export GBDK_HOME=/opt/gbdk/
cd template_minimal
make
```

## Examples

| Folder | Demonstrates |
|--------|-------------|
| `template_minimal/` | Minimal game loop, scaffold for a new project |
| `colorbar/` | CGB palettes, dual-bank VRAM tile/attribute split |
| `hblank_copy/` | HBlank ISR, fast VRAM animation with dual banking |
| `sound/` | All 4 audio channels, direct register control |
| `space/` | Sprites, scrolling, input — complete mini-game |

These examples are not integrated into the CMake build. They are for reference only.
