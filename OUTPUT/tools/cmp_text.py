"""Compara las secciones de dos PE byte a byte e imprime cuántos bytes difieren por sección."""
import sys, pefile
a, b = pefile.PE(sys.argv[1]), pefile.PE(sys.argv[2])
sb = {s.Name: s for s in b.sections}
for s in a.sections:
    o = sb.get(s.Name)
    if not o: print(s.Name, "solo en A"); continue
    da, db = s.get_data(), o.get_data()
    diff = [i for i in range(min(len(da), len(db))) if da[i] != db[i]]
    print(s.Name.rstrip(b'\0').decode(), "len", len(da), len(db), "diff", len(diff), [hex(s.VirtualAddress + i + 0x400000) for i in diff[:12]])
