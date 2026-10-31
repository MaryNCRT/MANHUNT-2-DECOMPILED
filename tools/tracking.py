"""Function tracking for the decomp: builds decomp/functions.tsv and measures progress.

decomp/functions.tsv has one row per function of the original executable:

    addr      entry address (hex, no 0x)
    size      body size in bytes
    section   .text, or .secu for game code that SecuROM relocated
    category  game | secu | dxut | crt | import | eh
    status    todo | readable | verified | replaced | dropped
    name      our name for it (empty until known)
    file      source file in decomp/ or runtime/ that holds it

Categories:
    game, secu  game code; counts toward decompilation progress
    dxut        DirectX SDK sample framework; replaced by runtime/, not decompiled
    crt         MSVC runtime/library identified by Ghidra FID; the compiler provides it
    import      thunks to DLL imports
    eh          compiler-generated exception funclets (follow their parent function)

Statuses:
    todo      not started
    readable  in decomp/ as named, typed, readable code
    verified  readable and checked against the original's behavior
    replaced  intentionally reimplemented by the port (runtime/)
    dropped   intentionally removed (DRM leftovers, affinity lock, ...)

Usage:
    python tools/tracking.py init      # (re)build from OUTPUT/inventory/functions.tsv, keeping statuses
    python tools/tracking.py report    # print progress and update README/PROGRESS tables
"""

import csv
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
TRACK = os.path.join(ROOT, "decomp", "functions.tsv")
INVENTORY = os.path.join(ROOT, "OUTPUT", "inventory", "functions.tsv")
FIELDS = ["addr", "size", "section", "category", "status", "name", "file"]

SECU_START, SECU_END = 0x7A5000, 0x7D9000
AUTO_NAME = re.compile(r"^(FUN_|thunk_FUN_|caseD_|default$|switchD|LAB_)")
DECOMP_CATEGORIES = ("game", "secu")
DONE = ("readable", "verified", "replaced", "dropped")


def load(path):
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def save(rows):
    with open(TRACK, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, FIELDS, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def classify(addr, ghidra_name):
    a = int(addr, 16)
    if ghidra_name.startswith(("Catch@", "Catch_All@", "Unwind@")):
        return "eh"
    if not AUTO_NAME.match(ghidra_name):
        # FID names MSVC runtime routines with a leading underscore; the rest
        # are D3DX/FMOD/Bink import thunks named after the imported symbol.
        return "crt" if ghidra_name.startswith("_") and "@" not in ghidra_name else "import"
    return "secu" if SECU_START <= a < SECU_END else "game"


def init():
    old = {r["addr"]: r for r in load(TRACK)} if os.path.exists(TRACK) else {}
    rows = []
    for r in load(INVENTORY):
        addr = r["addr"].lower()
        a = int(addr, 16)
        prev = old.get(addr)
        rows.append({
            "addr": addr,
            "size": r["bytes"],
            "section": ".secu" if SECU_START <= a < SECU_END else ".text",
            "category": prev["category"] if prev else classify(addr, r["name"]),
            "status": prev["status"] if prev else "todo",
            "name": prev["name"] if prev else ("" if AUTO_NAME.match(r["name"]) else r["name"]),
            "file": prev["file"] if prev else "",
        })
    save(rows)
    print(f"{len(rows)} functions -> {os.path.relpath(TRACK, ROOT)}")


def pct(part, whole):
    return 100.0 * part / whole if whole else 0.0


def measure(rows):
    m = {}
    game = [r for r in rows if r["category"] in DECOMP_CATEGORIES]
    gbytes = sum(int(r["size"]) for r in game)
    m["game_fns"], m["game_bytes"] = len(game), gbytes
    for st in ("readable", "verified"):
        sel = [r for r in game if r["status"] == st]
        m[st + "_fns"] = len(sel)
        m[st + "_bytes"] = sum(int(r["size"]) for r in sel)
    done = m["readable_bytes"] + m["verified_bytes"]
    m["decomp_pct"] = pct(done, gbytes)
    m["verified_pct"] = pct(m["verified_bytes"], gbytes)
    # Tentative names ending in "(?)" do not count as named.
    m["named_fns"] = sum(1 for r in game if r["name"] and not r["name"].endswith("(?)"))
    dx = [r for r in rows if r["category"] == "dxut"]
    m["dxut_fns"] = len(dx)
    m["dxut_replaced"] = sum(1 for r in dx if r["status"] == "replaced")
    m["total_fns"] = len(rows)
    m["by_cat"] = {}
    for r in rows:
        c = m["by_cat"].setdefault(r["category"], [0, 0])
        c[0] += 1
        c[1] += int(r["size"])
    return m


PHASES = os.path.join(ROOT, "docs", "phases.tsv")
BAR_WIDTH = 20
PARTIAL = " ▏▎▍▌▋▊▉"


def bar(p):
    """Text progress bar with 1/8-character resolution. No rounding up: an
    empty bar means less than 1/160 of the work is done."""
    eighths = int(max(0.0, min(100.0, p)) / 100.0 * BAR_WIDTH * 8)
    full, part = divmod(eighths, 8)
    s = "█" * full + (PARTIAL[part] if part else "")
    return "`" + s + "░" * (BAR_WIDTH - len(s)) + "`"


def fmt_pct(p):
    return f"{p:.2f} %" if p < 10 else f"{p:.1f} %"


def table(m):
    readable = m["readable_fns"] + m["verified_fns"]
    named_pct = pct(m["named_fns"], m["game_fns"])
    dxut_pct = pct(m["dxut_replaced"], m["dxut_fns"])
    return "\n".join([
        "| Metric (measured by `tools/tracking.py`) | Progress | % | Count |",
        "|---|---|---|---|",
        f"| Game code decompiled to readable C/C++ (by bytes) | {bar(m['decomp_pct'])} | "
        f"**{fmt_pct(m['decomp_pct'])}** | {readable} of {m['game_fns']:,} functions |",
        f"| Game code verified against the original (by bytes) | {bar(m['verified_pct'])} | "
        f"**{fmt_pct(m['verified_pct'])}** | {m['verified_fns']} functions |",
        f"| Game functions with a confirmed name | {bar(named_pct)} | {fmt_pct(named_pct)} | "
        f"{m['named_fns']} of {m['game_fns']:,} |",
        f"| DXUT functions replaced by the port | {bar(dxut_pct)} | {fmt_pct(dxut_pct)} | "
        f"{m['dxut_replaced']} of {m['dxut_fns']} identified |",
        "",
        f"Functions in the executable: {m['total_fns']:,}. CRT, DXUT, import thunks and "
        "exception funclets are not part of the game-code total.",
    ])


def phase_table(m):
    if not os.path.exists(PHASES):
        return ""
    measured = {"decomp": m["decomp_pct"], "verified": m["verified_pct"]}
    lines = ["| # | Phase | Progress | % | M/E | Basis |", "|---|---|---|---|---|---|"]
    for i, r in enumerate(load(PHASES)):
        p = measured[r["pct"]] if r["kind"] == "M" else float(r["pct"])
        lines.append(f"| {i} | {r['phase']} | {bar(p)} | {fmt_pct(p)} | {r['kind']} | {r['how']} |")
    lines += ["", "**M** = measured from `decomp/functions.tsv`. **E** = estimate, kept "
              "deliberately conservative, for phases with no countable unit yet "
              "(edit `docs/phases.tsv`)."]
    return "\n".join(lines)


def splice(path, marker, block):
    if not os.path.exists(path):
        return
    s = open(path, encoding="utf-8").read()
    new = re.sub(rf"(<!-- {marker}:start -->\n).*?(\n?<!-- {marker}:end -->)",
                 lambda mm: mm.group(1) + block + "\n<!-- " + marker + ":end -->", s, flags=re.S)
    if new != s:
        open(path, "w", encoding="utf-8", newline="\n").write(new)


def report():
    m = measure(load(TRACK))
    t, ph = table(m), phase_table(m)
    print(t)
    print()
    print(ph)
    print()
    for cat, (n, b) in sorted(m["by_cat"].items()):
        print(f"  {cat:7} {n:6} fns {b:9} B")
    for doc in ("README.md", "README_es.md", "PROGRESS.md"):
        splice(os.path.join(ROOT, doc), "progress", t)
        splice(os.path.join(ROOT, doc), "phases", ph)


if __name__ == "__main__":
    {"init": init, "report": report}[sys.argv[1] if len(sys.argv) > 1 else "report"]()
