"""Package only binaries whose checks match the current sources and bytes."""
import hashlib
import json
import zipfile
from datetime import datetime, timezone
from common import ROOT, VERSION, SDK_COMMIT, IMAGE, source_hash, sha256

dist = ROOT / "dist"
host = json.loads((dist / "tests-host.json").read_text(encoding="utf-8"))
target = json.loads((dist / "tests-target.json").read_text(encoding="utf-8"))
fingerprint = source_hash()
assert host["status"] == target["status"] == "passed", "Checks must pass"
assert host["source_sha256"] == target["source_sha256"] == fingerprint, "Stale checks: rerun make test and make hh3"
assert target["binary_sha256"] == sha256(dist / "mathe.hh3"), "Binary changed after checks"
assert target["ram_region"]["name"] == "VBAK" and target["startup_hooks"], "Missing memory/startup verification"
metadata = {
    "name": "Mathe Vektor", "version": VERSION,
    "target": "Casio ClassPad II / Hollyhock 3 / Custom Launcher v0.5 / OS 02.01.7002.0001",
    "created_utc": datetime.now(timezone.utc).isoformat(),
    "sdk_commit": SDK_COMMIT, "toolchain_image": IMAGE,
    "source_sha256": fingerprint,
    "binary_sha256": target["binary_sha256"],
    "host_checks": host, "target_checks": target,
    "hardware_tested": False,
}
build_data = (json.dumps(metadata, indent=2) + "\n").encode("utf-8")
(dist / "BUILD.json").write_bytes(build_data)


def write_zip(path, entries):
    sums = "".join(hashlib.sha256(data).hexdigest() + "  " + name + "\n" for name, data in sorted(entries.items()))
    entries = dict(entries)
    entries["SHA256SUMS.txt"] = sums.encode("ascii")
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(entries.items()):
            archive.writestr(name, data)
    with zipfile.ZipFile(path) as archive:
        assert archive.testzip() is None
        for name, data in entries.items():
            assert archive.read(name) == data


source_entries = {}
for name in ("Makefile", "README.md", "LICENSE", "build.ps1", ".gitignore", ".gitattributes"):
    source_entries[name] = (ROOT / name).read_bytes()
for folder in ("src", "tests", "tools", "docs"):
    for path in sorted((ROOT / folder).rglob("*")):
        if path.is_file() and "__pycache__" not in path.parts and "preview" not in path.parts and path.suffix not in (".ppm", ".pyc"):
            source_entries[path.relative_to(ROOT).as_posix()] = path.read_bytes()
sdk = ROOT / "vendor/hollyhock-3"
for path in sorted((sdk / "sdk").rglob("*")):
    if path.is_file() and path.suffix not in (".o", ".a"):
        source_entries[path.relative_to(ROOT).as_posix()] = path.read_bytes()
for name in ("README.md", "LICENSE"):
    source_entries["vendor/hollyhock-3/" + name] = (sdk / name).read_bytes()
source_entries["vendor/hollyhock-3/REVISION.txt"] = (SDK_COMMIT + "\n").encode("ascii")
suffix = VERSION.replace(".", "_")
source_path = dist / ("mathe_source_v" + suffix + ".zip")
write_zip(source_path, source_entries)

app_entries = {
    "mathe.hh3": (dist / "mathe.hh3").read_bytes(),
    "INSTALLATION.txt": (ROOT / "docs/INSTALLATION.txt").read_bytes(),
    "LICENSE": (ROOT / "LICENSE").read_bytes(),
    "FONT-LICENSE.txt": (ROOT / "docs/FONT-LICENSE.txt").read_bytes(),
}
app_path = dist / ("mathe_classpad_v" + suffix + ".zip")
write_zip(app_path, app_entries)
(dist / "SHA256SUMS.txt").write_text("".join(sha256(p) + "  " + p.name + "\n" for p in (dist / "mathe.hh3", app_path, source_path)), encoding="ascii")
print(f"Verified installation ZIP: {app_path}")
print(f"Verified source ZIP: {source_path}")
print(f"Checks: {host['checks']}; functions: {host['operations']}")
