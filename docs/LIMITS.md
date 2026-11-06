# Limits of the original game

Every fixed limit found in the 2009 executable, where it lives, and what the port
does instead. A limit is only listed once it has been seen in the code; suspected
limits go in the "To investigate" table.

Rule for the port: **raising a limit must never change how an original file is
read.** Original data and original mods must load exactly as before.

## Confirmed

| # | Limit | Where | Value | Effect | Port |
|---|---|---|---|---|---|
| L1 | Address space | PE header | `LARGE_ADDRESS_AWARE` off | Process limited to 2 GB; HD texture packs run out of memory | x64 build; LAA on in the x86 build |
| L2 | FMOD memory pool | `Audio_Init` 0x004CE4E0 | 15 MB (`0xF00000`), one fixed block | Every FMOD allocation (event data, streams, DSP) shares it; bigger or extra audio banks exhaust it | Pool sized from the loaded banks, or FMOD's own allocator |
| L4 | One packed file at a time | `File_Open` 0x0053BD00 | A single global reader (0x006A0390) with one `z_stream` | Opening a second `.glg` while one is being read resets the first one's stream | One reader object per open file |
| L3 | Text script size | `TextScript_Parse` 0x00541F90 | 80,000 bytes (`0x13880`): allocated, and at most that many read | Init scripts such as the level list (`global/initscripts/resource23.glg`, unpacked) are cut at 80,000 bytes; a mod adding levels or entries past that loses them silently | Buffer sized from the file |

## Bugs in the original

| # | Bug | Where | Port |
|---|---|---|---|
| B1 | **One byte written past the text buffer** when a script is 80,000 bytes or longer: after reading, it stores a space at `buffer[bytesRead]` (`mov byte [ebx+esi], 0x20` at 0x00542074, `ebx` = bytes read, set at 0x00541FD5), and `bytesRead` can equal the 80,000-byte allocation. Whether the allocator's rounding hides it in practice is not known. | `TextScript_Parse` | Buffer one byte larger than the file, and L3 removed |

## Found, meaning not yet known

| # | What | Where | Value |
|---|---|---|---|
| U1 | Fixed array of audio records, 0x38 bytes each | reset in `Audio_Init`, fields at 0x0070480C | 10 records |
| U2 | Multisample modes list | `0x00606E40`, array at 0x0079BEE8 | 4 entries (2, 4, 8, 16) |
| U3 | Fixed pool of records, 0x54 bytes each, linked as a free list | `0x005F1E90`, at 0x007953FC | 32 records |
| U4 | Buffer allocated once and zeroed | `0x005B9BD0`, pointer at 0x0077E854 | 1 MB (`0x100000`) |
| U5 | Buffer allocated at start-up | `0x00608680`, pointer at 0x0079D0F4 | 48 KB (`0xC000`) |

## To investigate

| Suspected limit | Lead |
|---|---|
| Texture count | `Manhunt2.NoTexureLimit.asi` exists, so the limit is real; location unknown |
| Static pools | `.data` has ~1.1 MB of zero-initialized space (BSS) where fixed pools usually live |
| 480x272 / 512x320 surfaces | `0x0060D420` sets PSP-sized surfaces; check whether anything is rendered at that size on PC |
