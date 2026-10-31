"""Disassembles one function of the reference exe with import, string and call annotations.

    python OUTPUT/tools/disasm.py ORIGINAL/Manhunt2_nosecu.exe 0x6154f0 198
"""
import sys, re, pefile, capstone

pe = pefile.PE(sys.argv[1]); base = pe.OPTIONAL_HEADER.ImageBase
start, size = int(sys.argv[2], 16), int(sys.argv[3])
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
for i in md.disasm(pe.get_data(start - base, size), start):
    note = ""
    for m in re.finditer(r"0x([0-9a-f]{6,8})", i.op_str):
        v = int(m.group(1), 16)
        if v in iat:
            note = iat[v]
        elif (s := string_at(v)):
            note = repr(s)
    print(f"{i.address:08x}  {i.mnemonic:6} {i.op_str:40} {('; ' + note) if note else ''}")
