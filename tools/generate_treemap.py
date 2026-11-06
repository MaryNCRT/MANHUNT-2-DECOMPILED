"""Byte-weighted decompilation treemap, drawn by BinViz from decomp/functions.tsv.

decomp/functions.tsv stays the source of truth. This script only translates it
into a BinViz notes file and asks BinViz to draw it:

    1. read and validate decomp/functions.tsv (against the Ghidra inventory too,
       when OUTPUT/inventory/functions.tsv exists)
    2. write OUTPUT/binviz/Manhunt2_nosecu.exe.binviz-notes.json
    3. run  binviz progress <exe> --notes <notes> --svg docs/progress/decompilation.svg
    4. check the SVG exists and that BinViz's totals agree with ours

    python tools/generate_treemap.py            # treemap only
    python tools/generate_treemap.py --report   # tracking.py report first, then the treemap

State mapping (BinViz semantics read from its source, crates/binviz/src/model.rs):

    ours                     BinViz decomp.state   why
    verified                 nonmatching           "its C does the same, but doesn't compile
                                                   to the same bytes": what the oracle proves
    readable                 in-progress           C exists, behavior not proven yet
    replaced                 library               reimplemented by the port, not decompiled
    dropped                  skipped               removed on purpose
    todo                     todo
    category crt/import/eh/dxut/zlib               library (outside game totals, like ours)

Nothing is ever "matched": in BinViz that means the C compiles to the very same
bytes, which this project does not attempt. BinViz's headline percentage counts
only matched bytes, so it reads 0 % here by design; our readable/verified
percentages come from tools/tracking.py.

Extents: Ghidra's sizes include non-contiguous pieces of a function, while a
BinViz note's address + size is one contiguous block. Where a function would
overlap the next one, the note's size is cut at the next function's address
and the full Ghidra size is kept in the comment.
"""

import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tracking import FIELDS, ROOT, TRACK, load, measure  # noqa: E402

EXE = os.path.join(ROOT, "ORIGINAL", "Manhunt2_nosecu.exe")
INVENTORY = os.path.join(ROOT, "OUTPUT", "inventory", "functions.tsv")
NOTES = os.path.join(ROOT, "OUTPUT", "binviz", "Manhunt2_nosecu.exe.binviz-notes.json")
SVG = os.path.join(ROOT, "docs", "progress", "decompilation.svg")
BINVIZ_CANDIDATES = [
    os.environ.get("BINVIZ", ""),
    os.path.join(ROOT, "TOOLS", "binviz", "release", "binviz.exe"),
    shutil.which("binviz") or "",
]

CATEGORIES = {"game", "secu", "dxut", "crt", "import", "eh", "zlib"}
STATUSES = {"todo", "readable", "verified", "replaced", "dropped"}
LIBRARY_CATEGORIES = {"crt", "import", "eh", "dxut", "zlib"}
STATE = {
    "verified": "nonmatching",
    "readable": "in-progress",
    "replaced": "library",
    "dropped": "skipped",
    "todo": "todo",
}


def fail(msg):
    sys.exit(f"generate_treemap: {msg}")


def validate(rows):
    seen = set()
    for i, r in enumerate(rows, 2):
        if set(r) != set(FIELDS):
            fail(f"line {i}: columns {sorted(r)} differ from {FIELDS}")
        if not re.fullmatch(r"[0-9a-f]{8}", r["addr"]):
            fail(f"line {i}: bad address {r['addr']!r}")
        if r["addr"] in seen:
            fail(f"line {i}: duplicate address {r['addr']}")
        seen.add(r["addr"])
        if not r["size"].isdigit() or int(r["size"]) <= 0:
            fail(f"line {i}: bad size {r['size']!r}")
        if r["category"] not in CATEGORIES:
            fail(f"line {i}: unknown category {r['category']!r}")
        if r["status"] not in STATUSES:
            fail(f"line {i}: unknown status {r['status']!r}")
    if os.path.exists(INVENTORY):
        with open(INVENTORY, encoding="utf-8", newline="") as f:
            inv = {r["addr"].lower() for r in csv.DictReader(f, delimiter="\t")}
        missing, extra = inv - seen, seen - inv
        if missing or extra:
            fail(f"tracking file and Ghidra inventory differ: {len(missing)} missing, {len(extra)} extra")
        print(f"validated {len(rows)} functions against the Ghidra inventory ({len(inv)})")
    else:
        print(f"validated {len(rows)} functions (no Ghidra inventory to compare with)")


def build_notes(rows):
    rows = sorted(rows, key=lambda r: int(r["addr"], 16))
    notes, clipped, clipped_bytes = [], 0, 0
    for i, r in enumerate(rows):
        addr, size = int(r["addr"], 16), int(r["size"])
        span = size
        if i + 1 < len(rows):
            gap = int(rows[i + 1]["addr"], 16) - addr
            if size > gap:
                span, clipped, clipped_bytes = gap, clipped + 1, clipped_bytes + size - gap
        library = r["category"] in LIBRARY_CATEGORIES
        state = "library" if library else STATE[r["status"]]
        comment = [f"category {r['category']}", f"status {r['status']}"]
        if r["name"].endswith("(?)"):
            comment.append(f"tentative name {r['name']}")
        if span != size:
            comment.append(f"Ghidra size {size} B incl. non-contiguous parts; drawn as {span} B")
        note = {
            "address": f"0x{addr:08x}",
            "size": f"0x{span:x}",
            "kind": "function",
            "comment": "; ".join(comment),
            # Only functions read against the disassembly count as reviewed.
            "reviewed": r["status"] in ("readable", "verified"),
            "decomp": {"state": state},
        }
        name = r["name"].removesuffix("(?)")
        if name:
            note["name"] = name
        if r["file"]:
            note["decomp"]["source"] = r["file"]
        notes.append(note)
    return notes, clipped, clipped_bytes


def find_binviz():
    for c in BINVIZ_CANDIDATES:
        if c and os.path.isfile(c):
            return c
    fail("binviz not found: build it (see docs/PROGRESS-TREEMAP.md) or set BINVIZ")


def compact_svg():
    """Drops each tile's hover tooltip, keeping its rectangle as drawn.

    GitHub shows the SVG as an image, so tooltips are never visible there, and
    with ~17,000 tiles they make the file several megabytes. Geometry, colors,
    legend and totals are untouched. The interactive version is BinViz's web UI.
    """
    text = open(SVG, encoding="utf-8").read()
    before = len(text)
    fills = lambda t: sorted(re.findall(r'<rect[^>]*fill="(#[0-9a-fA-F]{6})"', t))
    tiles_before = fills(text)
    # The title must not cross a tag, or a group that does not fit the pattern
    # (large tiles also carry a <text> label) would swallow the next one.
    text = re.sub(r"<g data-address=[^>]*>\s*<title>[^<]*</title>\s*(<rect[^>]*/>)\s*"
                  r"(<text[^>]*>[^<]*</text>)?\s*</g>", r"\1\2", text)
    # Every tile repeats the same outline: move it to one CSS rule.
    tile = ' stroke="#111b29" stroke-width="0.6"/>'
    text = text.replace(tile, ' class="t"/>')
    text = re.sub(r"(<svg[^>]*>)", r"\1<style>.t{stroke:#111b29;stroke-width:.6}</style>", text, count=1)
    # 4 decimals of a pixel are invisible; 1 is plenty.
    text = re.sub(r'="(\d+\.\d)\d+"', r'="\1"', text)
    if fills(text) != tiles_before:
        fail("compaction changed the set of tiles; SVG left as BinViz wrote it")
    with open(SVG, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print(f"compacted SVG: {before:,} -> {len(text):,} bytes")


def check_svg(rows, notes):
    if not os.path.isfile(SVG) or os.path.getsize(SVG) < 1000:
        fail("BinViz did not write the SVG")
    text = open(SVG, encoding="utf-8").read()
    m = re.search(r"(\d+) of (\d+) game functions matched; (\d+) of (\d+) game code bytes", text)
    if not m:
        fail("could not read BinViz's totals from the SVG")
    matched_fns, game_fns, matched_bytes, game_bytes = map(int, m.groups())
    ours_fns = sum(1 for n in notes if n["decomp"]["state"] != "library")
    ours_bytes = sum(int(n["size"], 16) for n in notes if n["decomp"]["state"] != "library")
    if matched_fns or matched_bytes:
        fail("BinViz reports matched code, but this project never records 'matched'")
    print(f"BinViz: {game_fns} game functions, {game_bytes} game bytes "
          f"(notes: {ours_fns} functions, {ours_bytes} bytes)")
    if game_fns != ours_fns or game_bytes != ours_bytes:
        print("  note: BinViz also counts code from its own inventory that our notes do not "
              "cover, or merges extents; difference "
              f"{game_fns - ours_fns:+d} functions, {game_bytes - ours_bytes:+d} bytes")
    m2 = measure(rows)
    print(f"tracking.py (unchanged): readable {m2['decomp_pct']:.2f} %, verified {m2['verified_pct']:.2f} %")


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--report", action="store_true", help="run tools/tracking.py report first")
    ap.add_argument("--width", type=int, default=1600)
    ap.add_argument("--height", type=int, default=900)
    a = ap.parse_args()

    if a.report:
        from tracking import report
        report()

    rows = load(TRACK)
    validate(rows)
    notes, clipped, clipped_bytes = build_notes(rows)
    os.makedirs(os.path.dirname(NOTES), exist_ok=True)
    with open(NOTES, "w", encoding="utf-8", newline="\n") as f:
        json.dump({"format": "binviz-annotations", "version": 1, "file": "Manhunt2.exe",
                   "annotations": notes}, f, indent=1)
    print(f"wrote {len(notes)} notes; {clipped} extents cut at the next function ({clipped_bytes} B)")

    if not os.path.isfile(EXE):
        fail(f"reference executable not found: {EXE}")
    # BinViz titles the image with the file name; a copy named after the game
    # (same bytes, in the unversioned OUTPUT/) gives a cleaner title.
    titled = os.path.join(os.path.dirname(NOTES), "Manhunt2.exe")
    if not os.path.isfile(titled) or os.path.getsize(titled) != os.path.getsize(EXE):
        shutil.copyfile(EXE, titled)
    os.makedirs(os.path.dirname(SVG), exist_ok=True)
    cmd = [find_binviz(), "progress", titled, "--notes", NOTES, "--svg", SVG,
           "--width", str(a.width), "--height", str(a.height), "--include-library"]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        fail(f"binviz failed ({r.returncode}): {r.stderr.strip() or r.stdout.strip()}")
    check_svg(rows, notes)
    compact_svg()
    print("wrote", os.path.relpath(SVG, ROOT))


if __name__ == "__main__":
    main()
