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

## Found, meaning not yet known

| # | What | Where | Value |
|---|---|---|---|
| U1 | Fixed array of audio records, 0x38 bytes each | reset in `Audio_Init`, fields at 0x0070480C | 10 records |
| U2 | Multisample modes list | `0x00606E40`, array at 0x0079BEE8 | 4 entries (2, 4, 8, 16) |

## To investigate

| Suspected limit | Lead |
|---|---|
| Texture count | `Manhunt2.NoTexureLimit.asi` exists, so the limit is real; location unknown |
| Static pools | `.data` has ~1.1 MB of zero-initialized space (BSS) where fixed pools usually live |
| 480x272 / 512x320 surfaces | `0x0060D420` sets PSP-sized surfaces; check whether anything is rendered at that size on PC |
