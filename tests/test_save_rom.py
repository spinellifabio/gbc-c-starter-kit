"""Runs tests/rom/test_save.c headless in BGB and checks its SRAM result block.
Needs the cmake build dir (cmake --preset ninja-gbdk) and BGB (env BGB_EXE to override)."""
import os
import re
import subprocess
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BGB = Path(os.environ.get("BGB_EXE", ROOT.parent / "emulator" / "bgbw64" / "bgb64.exe"))
ROM = ROOT / "build" / "test" / "test_save.gbc"
RESULT_OFFSET = 0x100  # keep in sync with RESULT_ADDR in tests/rom/test_save.c
# Order must match the CHECK() calls in tests/rom/test_save.c
CHECKS = [
    "erased slot is empty", "out-of-range write rejected", "write ok", "read is valid",
    "fields round-trip", "slots independent", "flipped byte -> corrupted",
    "bad version -> corrupted", "erase -> empty", "autosave read valid", "state round-trip",
]


@unittest.skipUnless(BGB.exists() and (ROOT / "build").exists(), "BGB or build dir missing")
class SaveRom(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        subprocess.run(["cmake", "--build", str(ROOT / "build"), "--target", "test_rom"],
                       check=True, capture_output=True)
        # sdld truncates map symbols to 9 chars: "_test_don"
        addr = re.search(r"([0-9A-F]{8})\s+_test_don\b", ROM.with_suffix(".map").read_text()).group(1)
        sav = ROM.with_suffix(".sav")
        sav.unlink(missing_ok=True)
        subprocess.run([str(BGB), "-hf", "-br", addr[-4:], "-rom", str(ROM)], check=True, timeout=60)
        cls.sram = sav.read_bytes()[RESULT_OFFSET:]

    def test_all_checks_pass(self):
        self.assertEqual(self.sram[0], 0xA5, "test ROM did not finish")
        self.assertEqual(self.sram[1], len(CHECKS))
        for i, name in enumerate(CHECKS):
            with self.subTest(name):
                self.assertEqual(self.sram[2 + i], 1)


if __name__ == "__main__":
    unittest.main()
