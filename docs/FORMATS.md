# File formats

Formats of the game's data files as far as the code shows them. Only structure is
described here; no game data is copied.

## `.glg` — packed text scripts

Used for most init scripts and `.ini` data on PC (`global/initscripts/resourceNN.glg`,
`global/ini/resourceNN.glg`). Which file each one replaces is given by path B of its
record in the path table (see PROGRESS.md, "CApp_Init and the file path table").

| Offset | Size | Content |
|---|---|---|
| 0x00 | 4 | Magic `Z2HM` (`5A 32 48 4D`) |
| 0x04 | 4 | Unpacked size, little-endian |
| 0x08 | … | zlib stream (`78 DA` header seen) |

Checked on `global/initscripts/resource23.glg` of the retail install (578 bytes packed,
1,085 bytes unpacked).

**Reader** (`decomp/io/File.cpp`): `File_Open` (0x0053BD00) opens the file and reads 4
bytes. If they are not `Z2HM`, it rewinds and returns the plain file, so **any file
opened this way may be plain text even where the game expects a `.glg`**. Otherwise it
reads the unpacked size and starts zlib **1.1.4** (statically linked) on a single global
reader object (0x006A0390): 4 KB packed input buffer, 4 KB unpacked output buffer, and a
`z_stream` at `+0x2018`. The reader cannot seek (its `Seek` is a bare `ret 4`) and only
one packed file can be open at a time (limit L4).

Unpacked text is read by `TextScript_Parse` (0x00541F90): at most 80,000 bytes (limit L3),
UTF-16 with a BOM is accepted and folded to ASCII, `#` starts a comment outside quotes.

**For mods:** because of the plain-file fallback above, a `.glg` replaced by plain text
(same file name) should already load in the original game, as long as the text does not
start with `Z2HM` (not tested in game). The port keeps that and also accepts the path-B
name, so script mods need no packing tool.
