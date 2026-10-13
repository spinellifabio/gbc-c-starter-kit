"""Runs the tests/rom/test_*.c ROMs headless in BGB and checks their SRAM result block.
Needs the cmake build dir (cmake --preset ninja-gbdk) and BGB (env BGB_EXE to override)."""
import os
import re
import subprocess
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BGB = Path(os.environ.get("BGB_EXE", ROOT.parent / "emulator" / "bgbw64" / "bgb64.exe"))
RESULT_OFFSET = 0x100  # keep in sync with RESULT_ADDR in tests/rom/harness.h


@unittest.skipUnless(BGB.exists() and (ROOT / "build").exists(), "BGB or build dir missing")
class RomCase(unittest.TestCase):
    rom = None
    checks = ()  # names, in the order of the CHECK() calls in the ROM's C source

    @classmethod
    def setUpClass(cls):
        if cls.rom is None:
            raise unittest.SkipTest("base class")
        subprocess.run(["cmake", "--build", str(ROOT / "build"), "--target", "test_rom"],
                       check=True, capture_output=True)
        rom = ROOT / "build" / "test" / f"{cls.rom}.gbc"
        # sdld truncates map symbols to 9 chars: "_test_don"
        addr = re.search(r"([0-9A-F]{8})\s+_test_don\b", rom.with_suffix(".map").read_text()).group(1)
        sav = rom.with_suffix(".sav")
        sav.unlink(missing_ok=True)
        subprocess.run([str(BGB), "-hf", "-br", addr[-4:], "-rom", str(rom)], check=True, timeout=60)
        cls.sram = sav.read_bytes()[RESULT_OFFSET:]

    def test_all_checks_pass(self):
        self.assertEqual(self.sram[0], 0xA5, "test ROM did not finish")
        self.assertEqual(self.sram[1], len(self.checks))
        for i, name in enumerate(self.checks):
            with self.subTest(name):
                self.assertEqual(self.sram[2 + i], 1)


class Save(RomCase):
    rom = "test_save"
    checks = (
        "erased slot is empty", "out-of-range write rejected", "write ok", "read is valid",
        "fields round-trip", "slots independent", "flipped byte -> corrupted",
        "bad version -> corrupted", "erase -> empty", "autosave read valid", "state round-trip",
    )


class Utils(RomCase):
    rom = "test_utils"
    checks = (
        "int_to_str 0", "int_to_str 65535", "int_to_str zero-pad", "int_to_str min_digits < digits",
        "int_to_str inner zeros", "tile of space", "tile of A", "tile of ~",
        "tile below space -> space", "tile DEL -> space", "tile high bit -> space",
    )


class World(RomCase):
    rom = "test_world"
    checks = (
        "lose life 2->1", "lose life 1->0 signals game over", "no underflow, no repeat signal",
        "npc overlap", "npc left edge touch", "npc right edge touch", "npc 1px inside",
        "npc inactive slot", "npc index out of range", "npc_solid_at",
        "object hit", "inactive object ignored", "empty space no hit",
    )


if __name__ == "__main__":
    unittest.main()
