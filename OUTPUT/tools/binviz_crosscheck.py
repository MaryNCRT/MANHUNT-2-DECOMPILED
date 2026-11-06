"""Cross-checks Ghidra's function inventory against BinViz's own analysis.

Finds functions BinViz sees and Ghidra does not, and keeps only those with
strong evidence of being real functions:

    direct call          an E8 call in code targets the start
    pointer table        the address sits in .rdata/.data next to another
                         function address (vtable, callback or initializer table)
    address taken        a push/mov/lea in code uses the address

and that end like a function (ret, or a jmp: thunks and tail calls). Being the
target of a plain jmp is NOT enough: shared function tails (a lone "ret" other
code jumps to) look exactly like that.

Writes OUTPUT/binviz/create_functions.txt for CreateFunctions.java and prints
what was left out and why. Needs `binviz progress <exe> --notes <notes> json`
saved as OUTPUT/binviz/progress.json first (see docs/INVENTORY.md).

    python OUTPUT/tools/binviz_crosscheck.py
"""

import bisect
import collections
import csv
import json
import os
import re
import struct

import capstone
import pefile

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
EXE = os.path.join(ROOT, "ORIGINAL", "Manhunt2_nosecu.exe")
PROGRESS = os.path.join(ROOT, "OUTPUT", "binviz", "progress.json")
INVENTORY = os.path.join(ROOT, "OUTPUT", "inventory", "functions.tsv")
OUT = os.path.join(ROOT, "OUTPUT", "binviz", "create_functions.txt")
BASE = 0x400000
ENDS_LIKE_FUNCTION = {"ret", "tiny ret", "jmp thunk", "ends in jmp"}


def shape(pe, md, va, size):
    """What the bytes look like: padding, thunk, function ending in ret..."""
    b = pe.get_data(va - BASE, size)
    if set(b) <= {0xCC} or set(b) <= {0x90} or set(b) <= {0}:
        return "padding"
    ins = list(md.disasm(b, va))
    if sum(i.size for i in ins) < len(b):
        return "not code"
    m = [i.mnemonic for i in ins]
    if m == ["jmp"]:
        return "jmp thunk"
    if m[-1] == "ret":
        return "tiny ret" if len(m) <= 3 else "ret"
    if m[-1] == "jmp":
        return "ends in jmp"
    return "no end"


def main():
    pe = pefile.PE(EXE)
    img = pe.get_memory_mapped_image()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    bv = {}
    for u in json.load(open(PROGRESS))["units"]:
        for f in u["functions"]:
            bv[int(f["metadata"]["virtual_address"])] = int(f["size"])
    with open(INVENTORY, encoding="utf-8", newline="") as f:
        gh = {int(r["addr"], 16) for r in csv.DictReader(f, delimiter="\t")}
    extra = {a: s for a, s in bv.items() if a not in gh}
    known = gh | set(extra)
    print(f"BinViz {len(bv)}  Ghidra {len(gh)}  only in BinViz {len(extra)}  only in Ghidra {len(gh - set(bv))}")

    calls, table, taken = collections.Counter(), collections.Counter(), collections.Counter()
    hexre = re.compile(r"0x[0-9a-f]+")
    # Without skipdata, capstone stops at the first byte it cannot decode:
    # on .text that was 0.3 % of the section. Skipping data bytes keeps the
    # linear sweep going to the end (it resynchronizes after embedded data).
    md.skipdata = True
    # A sweep that lost sync after embedded data can produce fake instructions;
    # a call or a taken address only counts if its instruction lies inside a
    # function Ghidra already knows.
    with open(INVENTORY, encoding="utf-8", newline="") as f:
        spans = sorted((int(r["addr"], 16), int(r["bytes"])) for r in csv.DictReader(f, delimiter="\t"))
    span_starts = [a for a, _ in spans]

    def in_known_function(va):
        i = bisect.bisect_right(span_starts, va) - 1
        return i >= 0 and va < spans[i][0] + spans[i][1]
    for s in pe.sections:
        lo, size = s.VirtualAddress, s.Misc_VirtualSize
        name = s.Name.rstrip(b"\0")
        if s.Characteristics & 0x20000000:  # code
            for ins in md.disasm(bytes(img[lo:lo + size]), BASE + lo):
                if not in_known_function(ins.address):
                    continue
                if ins.mnemonic == "call" and ins.op_str.startswith("0x"):
                    t = int(ins.op_str, 16)
                    if t in extra:
                        calls[t] += 1
                elif ins.mnemonic in ("push", "mov", "lea"):
                    for tok in hexre.findall(ins.op_str):
                        if int(tok, 16) in extra:
                            taken[int(tok, 16)] += 1
        elif name in (b".rdata", b".data"):
            for i in range(lo, lo + size - 4, 4):
                v = struct.unpack_from("<I", img, i)[0]
                if v in extra:
                    prev = struct.unpack_from("<I", img, i - 4)[0] if i - 4 >= lo else 0
                    nxt = struct.unpack_from("<I", img, i + 4)[0]
                    if prev in known or nxt in known:
                        table[v] += 1

    keep, left_out = {}, collections.Counter()
    for a, size in sorted(extra.items()):
        k = shape(pe, md, a, size)
        if calls[a]:
            keep[a] = "direct call"
        elif k in ENDS_LIKE_FUNCTION and table[a]:
            keep[a] = "pointer table (.rdata/.data)"
        elif k in ENDS_LIKE_FUNCTION and taken[a]:
            keep[a] = "address taken in code"
        else:
            left_out[k] += 1

    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        for a, why in keep.items():
            f.write(f"0x{a:08x}\t{why}\n")
    print(f"kept {len(keep)} ({sum(extra[a] for a in keep)} B): "
          f"{dict(collections.Counter(keep.values()))}")
    print(f"left out {sum(left_out.values())}: {dict(left_out)}")
    print("wrote", os.path.relpath(OUT, ROOT))


if __name__ == "__main__":
    main()
