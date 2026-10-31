# PROGRESS.md — Manhunt 2 PC Decomp

Project state. Written so anyone can pick this up with no prior context.

**Last update:** 2026-10-31

---

## 0. Where everything is

| Path | What it is |
|---|---|
| `E:\MANHUNT 2 DESCOMP\` | Repo and working tree (remote `MaryNCRT/MANHUNT-2-DESCOMP`). `.gitignore` blocks binaries, assets and analysis output. |
| `MH2GAME\Manhunt 2\` | Retail game install. **Do not modify.** |
| `ORIGINAL\` | Reference copies of the executables (not versioned). |
| `REFERENCE\` | Community repos, shallow clones, read-only (not versioned). See §4. |
| `TOOLS\` | Ghidra 12.1.2, GhidraMCP, REA package (not versioned). |
| `E:\rea-rea-agents-6.3.0` | REA source. |

---

## 1. Status

### Measured (by `tools/tracking.py`, never edited by hand)

<!-- progress:start -->
| Metric (measured by `tools/tracking.py`) | Progress | % | Count |
|---|---|---|---|
| Game code decompiled to readable C/C++ (by bytes) | `░░░░░░░░░░░░░░░░░░░░` | **0.08 %** | 9 of 14,929 functions |
| Game code verified against the original (by bytes) | `░░░░░░░░░░░░░░░░░░░░` | **0.01 %** | 2 functions |
| Game functions with a confirmed name | `░░░░░░░░░░░░░░░░░░░░` | 0.07 % | 10 of 14,929 |
| DXUT functions replaced by the port | `░░░░░░░░░░░░░░░░░░░░` | 0.00 % | 0 of 20 identified |

Functions in the executable: 15,407. CRT, DXUT, import thunks and exception funclets are not part of the game-code total.
<!-- progress:end -->

### Per phase

<!-- phases:start -->
| # | Phase | Progress | % | M/E | Basis |
|---|---|---|---|---|---|
| 0 | Binary analysis and mapping | `█████░░░░░░░░░░░░░░░` | 25.0 % | E | headers, imports, DRM layout, entry, WinMain, DXUT and game init found; subsystems not mapped |
| 1 | Function inventory and library identification | `██████░░░░░░░░░░░░░░` | 30.0 % | E | inventory complete; 296 CRT (FID) and 20 DXUT functions identified; other static libraries not separated |
| 2 | Subsystem map (strings, vtables, RTTI) | `▌░░░░░░░░░░░░░░░░░░░` | 3.00 % | E | boot, audio, settings and clock entry points located |
| 3 | Catalog of original limits (docs/LIMITS.md) | `▌░░░░░░░░░░░░░░░░░░░` | 3.00 % | E | 2 limits found so far |
| 4 | Build skeleton x86 + x64 (CMake), boot with stubs | `░░░░░░░░░░░░░░░░░░░░` | 0.00 % | E | not started |
| 5 | Game data setup from installed folder or ISO | `░░░░░░░░░░░░░░░░░░░░` | 0.00 % | E | not started |
| 6 | Differential test harness (the oracle) | `████████░░░░░░░░░░░░` | 40.0 % | E | loader, import traps, runner and live-input rule work; 2 functions verified; callee stubs, thiscall helpers and side-effect diff missing |
| 7 | Decompilation: game code readable | `░░░░░░░░░░░░░░░░░░░░` | 0.08 % | M | measured by bytes of game code |
| 8 | Decompilation: game code verified | `░░░░░░░░░░░░░░░░░░░░` | 0.01 % | M | measured by bytes of game code |
| 9 | Original mod support (docs/MODS.md) | `▍░░░░░░░░░░░░░░░░░░░` | 2.00 % | E | plan written; 1 of 18 plugin features located; nothing implemented |
| 10 | Improvements: widescreen, FPS, XInput, modern renderer | `░░░░░░░░░░░░░░░░░░░░` | 0.00 % | E | not started |

**M** = measured from `decomp/functions.tsv`. **E** = estimate, kept deliberately conservative, for phases with no countable unit yet (edit `docs/phases.tsv`).
<!-- phases:end -->

**Overall: the decompilation itself is at the measured percentage above (well under
1 %). Nothing compiles. Nothing is playable.**

---

## 2. Verified facts

### The retail executable is SecuROM-protected
`MH2GAME\Manhunt 2\Manhunt2.exe` (9,748,480 B) has `.securom`, `Stext`, `Sdata`…
sections, and `.text` has entropy 7.99: **it is encrypted** and cannot be analyzed
as-is.

**Decision:** the project **does not break or document DRM removal**. The reference
is the unprotected exe the user already had in other installs (the one the whole
modding scene uses):

| Copy | SHA-1 | Notes |
|---|---|---|
| `E:\Manhunt 2 remaster\Manhunt 2\Manhunt2.exe` → `ORIGINAL\Manhunt2_nosecu.exe` | `9a7053ada233b3aeb2433509dbeba9c45b086cbc` | **Reference.** 3,231,744 B, no LAA, unpatched |
| `E:\mh2\game\Manhunt 2\Manhunt2.exe` | `e272e836d3183a5fd5c8713b6c69b61dfba9dc02` | Same + LAA flag + 85 B in `.text` and 14 B in `.rdata` patched by mods |
| `E:\Manhunt 2\Manhunt2.exe` | `8e6bf178637d8a0c869eb5e431b459126006965e` | Patched variant |

Same `TimeDateStamp` (`0x4992F67C`) and same `.text/.rdata/.data` layout as retail:
it is the same build.

### Header
- PE32 i386, GUI, MSVC **8.0 (VS2005)**, ImageBase `0x400000`, entry `0x61C2A1`
- **`LARGE_ADDRESS_AWARE = False`** → source of the 2 GB limit
- `DllCharacteristics = 0` (no ASLR/DEP/NX-compat)
- Sections: `.text` 0x23649A · `.rdata` 0x4CD38 · `.data` 0x11C024 (VS) / 0x59000 (raw:
  ~1.1 MB of BSS, where the static pools will live) · `.rsrc` · `.secu` (SecuROM leftover)
- PDB: `W:\Manhunt2_PC\Assets\DvdRoot\Manhunt2.pdb` — not available. **No symbols.**
- One source path survives: `W:\Manhunt2_PC\Code\src\ai\graph.cpp`

### Ghidra inventory (`OUTPUT\inventory\`)
- **15,407 functions**, 2,141,510 bytes of code, **7,624 strings**
- Size buckets: <16 B: 4,433 · <256 B: 8,975 · <2 KB: 1,934 · ≥2 KB: 65
- 2,577 functions with auto names (`Catch@…`, switch cases, thunks): the rest are `FUN_…`

### Imports
| DLL | Use |
|---|---|
| `d3dx9_35.dll` | Only 6 functions: effects (`D3DXCreateEffectEx`/`Pool`) and matrices. `d3d9` is loaded dynamically |
| `fmodex.dll` (29) + `fmod_event.dll` (14) | Audio: FMOD Ex + Event System |
| `binkw32.dll` (12) | Video |
| `DINPUT8.dll` | Input (DirectInput only, **no XInput**) |
| `WSOCK32.dll` | `send/recv` (debug/network leftovers; to investigate) |
| `ADVAPI32` (services, SIDs, registry) / `VERSION` | Mostly SecuROM leftovers; to confirm |

All of these exist for x64 (D3D9, FMOD Ex, Bink) or can be replaced: nothing blocks
the 64-bit build.

### Windows 10/11 problems found so far
| Problem | Where | Port fix |
|---|---|---|
| Forced to a single CPU core | `0x6154F0`: `SetProcessAffinityMask(GetCurrentProcess(), 1)` | Call removed (`runtime/stubs`) |
| 2 GB address space | PE header, LAA off | x64 build; LAA on the x86 build |
| Any FMOD start-up failure closes the game ("Sound Error"): no audio device, headset unplugged at boot, odd driver | `Audio_Init` 0x4CE4E0 → `FatalError` 0x4CE4B0 | Fall back to no-sound output and keep running |

Limits are catalogued in [docs/LIMITS.md](docs/LIMITS.md).

---

## 3. Decisions

1. **64-bit path:** the same source compiles for x86 and x64. No patching the exe for
   x64. Portability rules are in `CLAUDE.md §2`.
2. **Game data source:** the port is set up from an installed game folder **or** the
   disc ISO, read directly by our own code. The original exe is never needed.
3. **REA:** the source build (`E:\rea-rea-agents-6.3.0`) cannot build its native addon
   on Windows (`build:windows-native` requires Linux + MinGW). We use the npm package
   `rea-agents@6.3.0` in `TOOLS\rea`, which ships `rea-windows-x64.node`.
   `rea doctor --provider ghidra` passes every Ghidra check, including
   `ghidra-native_authority`. Registered as an MCP server in `.mcp.json`.
4. Community mods found in other installs (`Manhunt2.NoTexureLimit.asi`,
   `Mh2Modern.asi`, `Manhunt2.DebugMenu.asi`, `Manhunt2.UM2.asi`…) are **hints for which
   limits exist and where**. Their code is not copied.
5. Repo language is English; `README_es.md` is the Spanish README.
6. **Treemap via BinViz** ([docs/PROGRESS-TREEMAP.md](docs/PROGRESS-TREEMAP.md)):
   `decomp/functions.tsv` stays the source of truth; `tools/generate_treemap.py`
   turns it into BinViz notes and renders `docs/progress/decompilation.svg`.
   `verified` → BinViz *nonmatching*, `readable` → *in-progress*; nothing is ever
   *matched* (byte-identical), so BinViz's headline and the decompilati.onl
   "Matching" badge read 0 % by design. BinViz is built locally with Rust into
   `TOOLS/binviz` (no prebuilt releases exist). No GitHub Action: rendering needs the
   original exe, which never enters the repo.

---

## 4. Community references (`REFERENCE\`, read-only)

| Repo | What it gives us |
|---|---|
| `winterheart/Manhunt2FSBanker` | FSB audio bank format (C++) |
| `spicybung/BLeeds` | Blender I/O for R* Leeds formats: `.mdl`, `.tex`/`.xtx`/`.chk`, anims (GPLv3) |
| `spicybung/Storyland` | Viewer/editor for MDL, TEX, COL2, IMG, LVZ, ANIM… (GPLv3) |
| `spicybung/Allens-MH2-Scripts` | IFP animation importer/exporter |
| `Blackbird88/mh2-affinity` | Single-core affinity fix → located in our exe (§2) |
| `LaurieWired/GhidraMCP` | Alternative Ghidra MCP plugin |

Documentation links (to read when each subsystem comes up):
- MDL format: https://sor3nt.github.io/MH2_MDL_Format_Documentation.html
- Script/level info: https://www.dixmor-hospital.com/mhs/index.php
- Plugin mods: https://www.dixmor-hospital.com/mods/plugin/
- Known PC issues: https://www.pcgamingwiki.com/wiki/Manhunt_2
- Also available if needed: angr, x64dbg

---

## 5. Decompilation log

| Date | Functions | Status | Notes |
|---|---|---|---|
| 2026-10-31 | `WinMain` 0x6155C0, `CountProcessesNamed` 0x615420 → `decomp/core/WinMain.cpp` | readable | Return value of 0x615420 confirmed in the disassembly (Ghidra lost it). Arity of 0x618EB0 unresolved. |
| 2026-10-31 | `MH2_InitGame` 0x6154F0 → `decomp/core/GameInit.cpp` | readable | Call order and `ECX` of every call read from the disassembly. Most callees unnamed. |
| 2026-10-31 | `Audio_Init` 0x4CE4E0, `Audio_TrackPeakMemory` 0x4CDEB0, `FatalError` 0x4CE4B0, sound error text/caption 0x40DC68 / 0x4CE4A0 → `decomp/audio/AudioInit.cpp` | readable | Mirrors FMOD Ex sample start-up; FMOD arguments from the disassembly (Ghidra's were wrong). Found limit L2 (15 MB FMOD pool). 0x4CDEB0 was assumed to be an error handler; it only tracks peak memory. |

| 2026-10-31 | `AlignUp64` 0x60DA60, `CountProcessesNamed` 0x615420 | **verified** | First functions verified by the oracle (`tests/oracle/`): 1,131,072 and 220 cases, 0 divergences, three runs. |

### The oracle
`tests/oracle/` runs the original code in-process: a placeholder exe linked at
0x400000 hands its address range to `oracle.dll`, which maps the hash-checked
original there, binds system imports and traps all others. Details and results in
[docs/METHODOLOGY.md](docs/METHODOLOGY.md).

### Game init sequence (`MH2_InitGame`)
Affinity lock → 0x53ABD0 → block layout 0x60DA80 → multisample probe (2/4/8/16) →
PSP-sized surface setup 0x60D420 (480x272 / 512x320) → main game object at 0x79E728 →
clock start → `Settings_ClampDetailLevel` → `Game_Init_004DF610` → **audio (FMOD)** →
`Game_Init_004DF450` (loads 4 things) → 0x4F8C40 → 0x4C5390 → sub-object vcall.

### Boot sequence (from WinMain)
1. Single-instance check: counts processes named `Manhunt2`, shows a localized
   "already running" box (EN/DE/FR/IT/ES) if more than one.
2. Command line: `-safe` (ignore saved settings), `-stereo` (stereo 3D mode).
3. Registers 11 game callbacks with **DXUT**.
4. `global/pictures/legal_fr.tex` present → French build.
5. Loads settings (or defaults), DXUT init / hotkeys / cursor / window "Manhunt 2".
6. Device creation (`0x40DC80`, a Rockstar-extended DXUTCreateDevice).
7. `MH2_InitGame` (`0x6154F0`): **affinity lock to 1 core**, then subsystem init.
8. DXUT main loop, then shutdown sequence (7 calls), exit code from DXUT.

## 6. Next work

0. BinViz's own analysis finds **2,512 functions (90,865 bytes) that Ghidra's inventory
   lacks**. Check whether they are real functions Ghidra missed; if so, add them to
   the inventory so the game-code total is complete.
1. Decompile `Game_Init_004DF450` and `Game_Init_004DF610` (what the game loads at boot)
2. Identify the rest of DXUT and the 11 game callbacks (render/update entry points)
3. Separate remaining static libraries (STL, zlib?, others)
4. Design and build the differential test harness (`docs/METHODOLOGY.md`)
5. Cross-check PCGamingWiki issues against the binary → table in §2
6. Limits catalog → `docs/LIMITS.md`
7. CMake skeleton x86/x64 → `runtime/`

---

## 7. Reproducing the analysis

```powershell
$env:JAVA_HOME = "C:\Program Files\Microsoft\jdk-21.0.12.8-hotspot"
TOOLS\ghidra_12.1.2_PUBLIC\support\analyzeHeadless.bat OUTPUT\ghidra MH2 `
    -import ORIGINAL\Manhunt2_nosecu.exe -overwrite `
    -scriptPath OUTPUT\tools -postScript ExportInventory.java OUTPUT\inventory
python OUTPUT\tools\peinfo.py ORIGINAL\Manhunt2_nosecu.exe
```

REA:
```powershell
cd TOOLS\rea; npm install rea-agents@6.3.0
```
