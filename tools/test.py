"""Run the sanitizer test binary and bind its result to the tested sources."""
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from common import ROOT, source_hash

out = ROOT / "dist/tests-host.json"
out.parent.mkdir(exist_ok=True)
fingerprint = source_hash()
out.write_text(json.dumps({"status": "running", "source_sha256": fingerprint}), encoding="utf-8")
result = subprocess.run([str(ROOT / "build/tests")], cwd=ROOT, capture_output=True, text=True)
layout = subprocess.run([sys.executable, "-m", "unittest", "discover", "-s", "tests", "-p", "test_binary.py"], cwd=ROOT, capture_output=True, text=True)
print(result.stdout, end="")
print(result.stderr, end="")
print(layout.stdout, end="")
print(layout.stderr, end="")
(ROOT / "dist/tests-host.log").write_text(result.stdout + result.stderr + layout.stdout + layout.stderr, encoding="utf-8")
match = re.search(r"PASS: (\d+) checks; (\d+) operations; (\d+) UI snapshots", result.stdout)
ok = result.returncode == 0 and layout.returncode == 0 and match is not None and source_hash() == fingerprint
data = {
    "status": "passed" if ok else "failed",
    "source_sha256": fingerprint,
    "completed_utc": datetime.now(timezone.utc).isoformat(),
    "checks": int(match[1]) if match else 0,
    "operations": int(match[2]) if match else 0,
    "ui_snapshots": int(match[3]) if match else 0,
    "layout_test_groups": 14,
    "sanitizers": ["AddressSanitizer", "UndefinedBehaviorSanitizer", "LeakSanitizer"],
    "hardware_tested": False,
}
out.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
raise SystemExit(0 if ok else 1)
