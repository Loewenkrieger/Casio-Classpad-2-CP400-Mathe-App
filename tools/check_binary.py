"""Check RAM boundaries, startup hooks, and the actual stripped HH3 image."""
import argparse
import json
from pathlib import Path
from common import ROOT, VERSION, SDK_COMMIT, IMAGE, source_hash, sha256
from elf_layout import APP_START, APP_END, inspect_elf, startup_hooks, require


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reject", type=Path, help="Prove that a known invalid old build is rejected by RAM checks")
    args = parser.parse_args()
    if args.reject:
        try:
            inspect_elf(args.reject.read_bytes())
        except ValueError as error:
            require("LOAD range outside VBAK" in str(error), "Old build failed a different check")
            data = {"status": "passed", "old_binary_rejected": True, "binary_sha256": sha256(args.reject), "reason": str(error)}
            (ROOT / "dist/tests-rejected-v1_0_0.json").write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
            print("PASS: invalid v1.0.0 rejected: " + str(error))
            return
        raise ValueError("Invalid old build unexpectedly passed")
    path = ROOT / "dist/mathe.hh3"
    blob = path.read_bytes()
    image = inspect_elf(blob)
    notes = image["notes"]
    require(notes.get("NAME") == "Mathe Vektor" and notes.get("VERSION") == VERSION, "Unexpected app/version")
    require(notes.get("AUTHOR") == "Loewenkrieger" and notes.get("DESCRIPTION"), "Missing metadata")
    full_blob = (ROOT / "dist/mathe.elf").read_bytes()
    hooks = startup_hooks(full_blob, image)
    footprint = max(s["vaddr"] + s["memory_bytes"] for s in image["loads"]) - APP_START
    data = {
        "status": "passed", "source_sha256": source_hash(), "binary_sha256": sha256(path),
        "file_bytes": len(blob), "entry_point": hex(image["entry"]), "format": "ELF32 big-endian SH4a-nofpu",
        "notes": notes, "load_segments": image["loads"],
        "ram_region": {"name": "VBAK", "start": hex(APP_START), "end_exclusive": hex(APP_END),
                       "capacity_bytes": APP_END - APP_START, "image_footprint_bytes": footprint,
                       "remaining_bytes": APP_END - APP_START - footprint},
        "startup_hooks": hooks, "screen_backup": "OS heap; strong calcInit/calcExit overrides verified in ELF initializer arrays",
        "sdk_commit": SDK_COMMIT, "toolchain_image": IMAGE, "hardware_tested": False,
    }
    (ROOT / "dist/tests-target.json").write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    print(f"PASS: HH3, VBAK bounds ({footprint}/{APP_END - APP_START} bytes), strong startup hooks and SDK initialization order")


if __name__ == "__main__":
    main()
