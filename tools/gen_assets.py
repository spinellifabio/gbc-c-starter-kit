#!/usr/bin/env python3
"""Regenerate res/ sources from assets/assets.json via png2asset.

Usage: python tools/gen_assets.py [name ...]   (no names = everything)

sprites  -> res/sprites/<name>.c/.h   (metasprites, 8x16 OAM)
tilesets -> res/tiles/<name>.c/.h     (png2asset flags taken from "flags")
Then prints the sprite VRAM tile budget over ALL res/sprites/*.h (manifest and
legacy). Entries not in the manifest are untouched.
"""
import json
import os
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
P2A = Path(os.environ.get("GBDK_ROOT", "C:/gbdk")) / "bin" / "png2asset.exe"
SPRITE_TILES = 256  # VRAM sprite tile slots (8x16 OAM mode)


def png_size(path):
    with open(path, "rb") as f:
        return struct.unpack(">II", f.read(24)[16:24])


def p2a(src, out, flags):
    cmd = [str(P2A), str(src), "-o", str(out), *flags]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode or not out.exists():
        sys.exit(f"png2asset failed: {' '.join(cmd)}\n{r.stdout}{r.stderr}"
                 "(silent failure usually = more than 4 colors in an 8x16 tile)")


def main():
    if not P2A.exists():
        sys.exit(f"png2asset not found at {P2A} (set GBDK_ROOT)")
    manifest = json.loads((ROOT / "assets" / "assets.json").read_text())
    only = set(sys.argv[1:])
    sprites, tilesets = manifest.get("sprites", {}), manifest.get("tilesets", {})
    unknown = only - sprites.keys() - tilesets.keys()
    if unknown:
        sys.exit(f"not in manifest: {', '.join(sorted(unknown))}")

    for name, s in sprites.items():
        if only and name not in only:
            continue
        src = ROOT / "assets" / s["src"]
        fw, fh = s["frame"]
        w, h = png_size(src)
        if w % fw or h % fh:
            sys.exit(f"{name}: {w}x{h} png is not a multiple of frame {fw}x{fh}")
        flags = ["-sw", fw, "-sh", fh, "-spr8x16"]
        if "pivot" in s:
            flags += ["-px", s["pivot"][0], "-py", s["pivot"][1]]
        if "hitbox" in s:
            flags += ["-pw", s["hitbox"][0], "-ph", s["hitbox"][1]]
        p2a(src, ROOT / "res" / "sprites" / f"{name}.c",
            [str(x) for x in flags + s.get("flags", [])])

    for name, t in tilesets.items():
        if not only or name in only:
            p2a(ROOT / "assets" / t["src"], ROOT / "res" / "tiles" / f"{name}.c",
                t.get("flags", []))

    # includes hand-managed legacy sprites (cat_idle, cutscene_*) that share the VRAM
    used = 0
    for h in sorted((ROOT / "res" / "sprites").glob("*.h")):
        n = int(re.search(r"_TILE_COUNT (\d+)", h.read_text()).group(1))
        print(f"  {h.stem:<20} {n:>3} tiles" + ("" if h.stem in sprites else "  (legacy)"))
        used += n
    # hand-written tile arrays (treasure_tiles[64] = 4 tiles x 16 bytes)
    for sym, size in re.findall(r"const uint8_t (\w+_tiles)\[(\d+)\]",
                                (ROOT / "src" / "game_objects.c").read_text()):
        print(f"  {sym:<20} {int(size) // 16:>3} tiles  (game_objects.c)")
        used += int(size) // 16
    print(f"sprite VRAM: {used}/{SPRITE_TILES}")
    if used > SPRITE_TILES:
        sys.exit("ERROR: sprite tiles exceed VRAM budget")


if __name__ == "__main__":
    main()
