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
1,085 bytes unpacked). The function that opens and unpacks it is not decompiled yet
(file open is 0x0053BD00).

Unpacked text is read by `TextScript_Parse` (0x00541F90): at most 80,000 bytes (limit L3),
UTF-16 with a BOM is accepted and folded to ASCII, `#` starts a comment outside quotes.

**For mods:** the port will accept, for any path-table slot, either the original `.glg`
or a plain text file with the path-B name, so script mods need no packing tool.
