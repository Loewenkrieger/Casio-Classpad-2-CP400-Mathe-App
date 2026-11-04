import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from elf_layout import APP_START, APP_END, check_load_ranges


def segment(start=APP_START, size=150000, files=149000, flags=5):
    return {"vaddr": start, "memory_bytes": size, "file_bytes": files, "flags": flags}


class LayoutTests(unittest.TestCase):
    def test_valid_large_app(self):
        check_load_ranges([segment()], APP_START + 64)

    def test_historical_v100_layout_rejected(self):
        with self.assertRaisesRegex(ValueError, "outside VBAK"):
            check_load_ranges([segment(0x8CFF0000, 150964, 150964), segment(0x8D014E34, 1668, 572, 6)], 0x8CFF1898)

    def test_exact_boundary(self):
        check_load_ranges([segment(size=APP_END - APP_START, files=APP_END - APP_START)], APP_START)

    def test_one_byte_overflow(self):
        with self.assertRaisesRegex(ValueError, "outside VBAK"):
            check_load_ranges([segment(size=APP_END - APP_START + 1)], APP_START)

    def test_live_vram_excluded(self):
        with self.assertRaisesRegex(ValueError, "outside VBAK"):
            check_load_ranges([segment(start=0x8C000000)], 0x8C000000)

    def test_heap_excluded(self):
        with self.assertRaisesRegex(ValueError, "outside VBAK"):
            check_load_ranges([segment(start=0x8C280000)], 0x8C280000)

    def test_launcher_excluded(self):
        with self.assertRaisesRegex(ValueError, "outside VBAK"):
            check_load_ranges([segment(start=0x8CFD0000)], 0x8CFD0000)

    def test_bss_and_two_segments(self):
        check_load_ranges([segment(size=149000, files=149000), segment(APP_START + 150000, 4000, 1000, 6)], APP_START)

    def test_bss_overflow(self):
        with self.assertRaisesRegex(ValueError, "outside VBAK"):
            check_load_ranges([segment(size=APP_END - APP_START + 1000, files=1000)], APP_START)

    def test_entry_not_in_bss(self):
        with self.assertRaisesRegex(ValueError, "executable bytes"):
            check_load_ranges([segment()], APP_START + 149500)

    def test_entry_not_in_data(self):
        with self.assertRaisesRegex(ValueError, "executable bytes"):
            check_load_ranges([segment(flags=6)], APP_START)

    def test_overlapping_segments(self):
        with self.assertRaisesRegex(ValueError, "Overlapping"):
            check_load_ranges([segment(), segment(APP_START + 4000, 3000, 3000, 6)], APP_START)

    def test_alignment(self):
        with self.assertRaisesRegex(ValueError, "Unaligned LOAD"):
            check_load_ranges([segment(start=APP_START + 1)], APP_START + 64)
        with self.assertRaisesRegex(ValueError, "Unaligned SH"):
            check_load_ranges([segment()], APP_START + 1)

    def test_bad_sizes(self):
        with self.assertRaisesRegex(ValueError, "Illegal LOAD sizes"):
            check_load_ranges([segment(size=4, files=8)], APP_START)
        with self.assertRaisesRegex(ValueError, "No LOAD"):
            check_load_ranges([], APP_START)


if __name__ == "__main__":
    unittest.main()
