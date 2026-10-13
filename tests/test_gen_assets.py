"""Asset pipeline checks. Run: python -m unittest discover -s tests (stdlib only)."""
import json
import re
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import gen_assets as ga  # noqa: E402

MANIFEST = json.loads((ga.ROOT / "assets" / "assets.json").read_text())


class PngSize(unittest.TestCase):
    def test_reads_ihdr_big_endian(self):
        hdr = b"\x89PNG\r\n\x1a\n" + struct.pack(">I", 13) + b"IHDR" + struct.pack(">II", 48, 96)
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "x.png"
            p.write_bytes(hdr)
            self.assertEqual(ga.png_size(p), (48, 96))


class Manifest(unittest.TestCase):
    def test_sprite_sheets_are_whole_frames(self):
        for name, s in MANIFEST["sprites"].items():
            w, h = ga.png_size(ga.ROOT / "assets" / s["src"])
            fw, fh = s["frame"]
            self.assertEqual((w % fw, h % fh), (0, 0), name)

    def test_tileset_sources_exist(self):
        for name, t in MANIFEST["tilesets"].items():
            self.assertTrue((ga.ROOT / "assets" / t["src"]).exists(), name)

    def test_sprite_vram_within_budget(self):
        used = sum(int(re.search(r"_TILE_COUNT (\d+)", h.read_text()).group(1))
                   for h in (ga.ROOT / "res" / "sprites").glob("*.h"))
        used += sum(int(n) // 16 for n in re.findall(
            r"const uint8_t \w+_tiles\[(\d+)\]", (ga.ROOT / "src" / "game_objects.c").read_text()))
        self.assertLessEqual(used, ga.SPRITE_TILES)


if __name__ == "__main__":
    unittest.main()
