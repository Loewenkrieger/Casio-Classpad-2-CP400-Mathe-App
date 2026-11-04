from pathlib import Path
import hashlib

ROOT = Path(__file__).resolve().parents[1]
VERSION = "1.0.3"
SDK_COMMIT = "1e483e395391d50d9bd2d21cee4beecc7589fb1c"
IMAGE = "ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a"


def source_hash():
    paths = sorted([ROOT / "Makefile", ROOT / "build.ps1", ROOT / "tools/build.sh",
                    *ROOT.glob("src/*.cpp"), *ROOT.glob("src/*.hpp"),
                    *ROOT.glob("tests/*.cpp"), *ROOT.glob("tests/*.py"), *ROOT.glob("tools/*.py")])
    digest = hashlib.sha256()
    for path in sorted(paths, key=lambda p: p.relative_to(ROOT).as_posix()):
        digest.update(path.relative_to(ROOT).as_posix().encode() + b"\0")
        digest.update(path.read_bytes())
    return digest.hexdigest()


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()
