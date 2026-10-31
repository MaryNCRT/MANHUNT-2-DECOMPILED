"""Set category/status/name/file for functions in decomp/functions.tsv.

    python tools/setstatus.py 6155c0 status=readable name=WinMain file=decomp/core/WinMain.cpp
    python tools/setstatus.py 40a590 4083d0 category=dxut
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tracking import FIELDS, load, save, TRACK  # noqa: E402

addrs = [a.lower().removeprefix("0x").zfill(8) for a in sys.argv[1:] if "=" not in a]
changes = dict(a.split("=", 1) for a in sys.argv[1:] if "=" in a)
bad = set(changes) - set(FIELDS)
if bad:
    sys.exit(f"unknown field(s): {', '.join(sorted(bad))}")

rows = load(TRACK)
found = 0
for r in rows:
    if r["addr"] in addrs:
        r.update(changes)
        found += 1
save(rows)
missing = len(addrs) - found
print(f"updated {found}" + (f", NOT FOUND {missing}" if missing else ""))
sys.exit(1 if missing else 0)
