"""Disassembles one function of the reference exe with import, string and call annotations.

    python OUTPUT/tools/disasm.py ORIGINAL/Manhunt2_nosecu.exe 0x6154f0 198
    python OUTPUT/tools/disasm.py ORIGINAL/Manhunt2_nosecu.exe 0x7d27f0 follow

"follow" reads in execution order instead of by size: it follows every
unconditional jmp to a fixed address (SecuROM split some functions into pieces
joined by jmps) and stops at a ret or an indirect jmp. Conditional branches are
printed but not followed; their targets are listed at the end.
"""
import sys, re, pefile, capstone

pe = pefile.PE(sys.argv[1]); base = pe.OPTIONAL_HEADER.ImageBase
start = int(sys.argv[2], 16)
size = 0 if sys.argv[3] == "follow" else int(sys.argv[3], 0)
iat = {imp.address: (imp.name.decode() if imp.name else f"#{imp.ordinal}")
       for e in pe.DIRECTORY_ENTRY_IMPORT for imp in e.imports}

def string_at(va):
    try:
        b = pe.get_data(va - base, 80)
    except Exception:
        return None
    a = b.split(b"\0")[0]
    if len(a) >= 4 and all(32 <= c < 127 for c in a):
        return a.decode()
    w = b.decode("utf-16le", "ignore").split("\0")[0]
    if len(w) >= 4 and w.isprintable():
        return "L" + w
    return None

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def show(i):
    note = ""
    for m in re.finditer(r"0x([0-9a-f]{6,8})", i.op_str):
        v = int(m.group(1), 16)
        if v in iat:
            note = iat[v]
        elif i.mnemonic not in ("call", "jmp") and (s := string_at(v)):
            note = repr(s)
    print(f"{i.address:08x}  {i.mnemonic:6} {i.op_str:40} {('; ' + note) if note else ''}")


if sys.argv[3] != "follow":
    for i in md.disasm(pe.get_data(start - base, size), start):
        show(i)
    sys.exit(0)

seen, branches, pc = set(), [], start
while pc not in seen:
    seen.add(pc)
    i = next(md.disasm(pe.get_data(pc - base, 16), pc), None)
    if i is None:
        print(f"{pc:08x}  (cannot decode)")
        break
    if i.mnemonic == "jmp" and i.op_str.startswith("0x"):
        pc = int(i.op_str, 16)  # piece boundary: continue at the target
        continue
    show(i)
    if i.mnemonic.startswith("j") and i.op_str.startswith("0x"):
        branches.append(int(i.op_str, 16))
    if i.mnemonic in ("ret", "retn") or i.mnemonic == "jmp":
        break
    pc = i.address + i.size
if branches:
    print("conditional branch targets:", " ".join(f"0x{b:08x}" for b in branches))
