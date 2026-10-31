"""PE summary: headers, sections, imports, flags and DRM/packer hints."""
import sys, math, collections, pefile

def entropy(b):
    if not b: return 0.0
    c = collections.Counter(b); n = len(b)
    return -sum(v/n*math.log2(v/n) for v in c.values())

pe = pefile.PE(sys.argv[1])
fh, oh = pe.FILE_HEADER, pe.OPTIONAL_HEADER
print(f"Machine=0x{fh.Machine:x} TimeDateStamp=0x{fh.TimeDateStamp:x} Characteristics=0x{fh.Characteristics:x}")
print(f"LARGE_ADDRESS_AWARE={bool(fh.Characteristics & 0x20)}")
print(f"ImageBase=0x{oh.ImageBase:x} EntryPoint=0x{oh.AddressOfEntryPoint:x} Subsystem={oh.Subsystem} "
      f"DllCharacteristics=0x{oh.DllCharacteristics:x} Linker={oh.MajorLinkerVersion}.{oh.MinorLinkerVersion}")
print("Sections:")
for s in pe.sections:
    print(f"  {s.Name.rstrip(b'\0').decode(errors='replace'):10} VA=0x{s.VirtualAddress:08x} VS=0x{s.Misc_VirtualSize:08x} "
          f"RS=0x{s.SizeOfRawData:08x} ch=0x{s.Characteristics:08x} H={entropy(s.get_data()):.2f}")
if hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
    for d in pe.DIRECTORY_ENTRY_IMPORT:
        names = [i.name.decode() if i.name else f"#{i.ordinal}" for i in d.imports]
        print(f"IMPORT {d.dll.decode()} ({len(names)}): {', '.join(names[:60])}{' ...' if len(names)>60 else ''}")
if hasattr(pe, "DIRECTORY_ENTRY_DEBUG"):
    for d in pe.DIRECTORY_ENTRY_DEBUG:
        e = d.entry
        if e and hasattr(e, "PdbFileName"): print("PDB:", e.PdbFileName.rstrip(b'\0').decode(errors='replace'))
data = open(sys.argv[1], "rb").read()
for tag in [b"SecuROM", b"securom", b".securom", b"SafeDisc", b"StarForce", b"Steam", b"steam_api", b"GFWL", b"xlive", b"Rich", b"RockstarGames", b"Rockstar"]:
    print(f"  marker {tag!r}: {data.count(tag)}")
