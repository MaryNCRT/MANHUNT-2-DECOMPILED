# CLAUDE.md — Manhunt 2 PC Decomp / PC Port

Native port of **Manhunt 2 (PC, Rockstar 2009)** to Windows 10/11, in **32-bit and
64-bit**, built from a decompilation of the original executable. Legally owned copy.

Repo: **https://github.com/MaryNCRT/MANHUNT-2-DESCOMP**

**Language:** everything in the repo (code, comments, docs, commits) is in
**English**. The only exception is `README_es.md`, the Spanish README.

---

## 0. Method: DECOMPILATION (same as UMK3)

We do not have the source code. The binary is **decompiled to readable C/C++**,
function by function, with Ghidra (headless + scripts) and **REA** as the
accelerator (inventory, function dossiers, value tracing, search).

The goal is not a patch on top of the exe: it is **our own compilable source code**
that replaces the whole executable, so mods have no limits.

### End goals
- Native **x86 (32-bit)** and **x64 (64-bit)** executables from the same source
- **No 2 GB limit** (native x64; on x86, LARGE_ADDRESS_AWARE from day one)
- **No fixed limits** on textures, pools, models, entities or buffers
- **HD models and textures**, mods loaded from folders (original files untouched)
- Runs on **Windows 10 / 11** with no third-party patches
- Later: correct widescreen and FOV, unlocked framerate with fixed-step logic,
  XInput controllers, raw mouse input, modern renderer

### Legal model (same as UMK3 / BattleShip)
**Zero assets and zero binaries in the repo.** The port is code only. Every user
supplies their own copy of the game. See `.gitignore`.

**Game data source:** the port is set up from either (a) an **existing installed game
folder**, or (b) the **game disc ISO**. Setup reads the data files directly from
either one (ISO 9660/UDF parsed by our own code, no mounting required) and never
uses the original executable.

**DRM:** the retail exe is SecuROM-protected. This project **does not include,
document or automate DRM removal.** Analysis uses the unprotected executable the
user already owned (see PROGRESS.md §2). The finished port does not need the
original exe at all, only the game data.

---

## 1. Autonomy

**Work continuously. Do not ask for approval between steps.** If a decision has a
reasonable default, take it, record it in `PROGRESS.md` and keep going.

Stop only if: (a) something truly blocks and there is no alternative, (b) an action
is destructive or irreversible, (c) a verified assumption turns out to be false.

Keep **`PROGRESS.md`** at the root, updated after each block of work: what was done,
what was verified, what is left, decisions taken and why. That file is the project
state. Write it for someone picking this up with no prior context.

---

## 2. Repo rules

- **Do not modify** `MH2GAME\`, `ORIGINAL\` or `REFERENCE\`: they are original sources.
- Readable decompiled code → `decomp/`
- Hand-written native code (platform, render, input, audio) → `runtime/`
- Stubs for removed code (SecuROM, affinity lock, etc.) → `runtime/stubs/`
- Tests and comparators → `tests/` · Analysis and dumps → `OUTPUT/` (not versioned)
- Our own tools (extractors, scripts) → `tools/` and `OUTPUT/tools/`
- **All code must compile for x86 and x64.** Never store pointers in `int`/`DWORD`;
  use `intptr_t`/`uintptr_t`. On-disk structs use fixed-size types (`uint32_t` etc.)
  with a `static_assert` on their size, and never contain pointers.
- **Identity:** the project owner is referred to only as **MaryNCRT**, everywhere
  (docs, notes, comments, commits, issues). Never write the owner's real name or
  personal email. Commits use `Mary N.C <69011077+MaryNCRT@users.noreply.github.com>`
  (set in the repo's local git config). Do not paste paths under the user profile
  (`C:\Users\...`) into versioned files.
- Community code in `REFERENCE\` is read for knowledge only. Do not copy it into the
  repo; GPL code in particular must never be pasted in.

---

## 3. Tools

| Tool | Location | Use |
|---|---|---|
| Ghidra 12.1.2 + JDK 21 | `TOOLS\ghidra_12.1.2_PUBLIC` · `C:\Program Files\Microsoft\jdk-21.0.12.8-hotspot` | Decompiler |
| REA 6.3.0 | `TOOLS\rea` (npm package with native addon) · source in `E:\rea-rea-agents-6.3.0` | RE MCP/CLI, Ghidra provider (`rea doctor` passes) |
| GhidraMCP | `TOOLS\GhidraMCP` | Ghidra over MCP (GUI) |
| `peinfo.py` | `OUTPUT\tools` | Headers, sections, imports, DRM markers |
| `cmp_text.py` | `OUTPUT\tools` | Per-section diff between two PEs |
| `ExportInventory.java` | `OUTPUT\tools` | Ghidra → `functions.tsv` / `strings.tsv` |
| `DecompileList.java` | `OUTPUT\tools` | Ghidra → C for a list of functions (with signatures/structs) |
| Ghidra project | `OUTPUT\ghidra\MH2.gpr` | Imported and analyzed binary |
| Oracle | `tests\oracle\build.ps1` (32-bit MSVC) | Runs original functions next to ours; zero divergences = verified |
| BinViz CLI | `TOOLS\binviz\release\binviz.exe` (built with Rust from `REFERENCE\binviz`) | Treemap via `tools\generate_treemap.py` |
| Progress | `python tools\generate_treemap.py --report` | Tables, bars and treemap in one command |
| Inventory cross-check | `OUTPUT\tools\binviz_crosscheck.py` + `CreateFunctions.java` | Adds functions Ghidra missed, only with strong evidence (`docs\INVENTORY.md`) |

MCP servers are registered in `.mcp.json` (machine-specific, not versioned).

---

## 4. Verified facts (do NOT re-discover)

See `PROGRESS.md §2`. Summary:
- PE32 i386, MSVC 8.0 (VS2005), ImageBase `0x400000`, TimeDateStamp `0x4992F67C` (2009-02-11)
- Original PDB `W:\Manhunt2_PC\Assets\DvdRoot\Manhunt2.pdb`: **not available**, binary has no symbols
- **16,720 functions** (15,407 from Ghidra auto-analysis + 1,313 confirmed by the BinViz cross-check), 7,624 strings
- Renderer **Direct3D 9** (d3dx9_35 + .fx effects), audio **FMOD Ex + FMOD Event**, video **Bink**,
  input **DirectInput 8**
- Forced single core: `SetProcessAffinityMask(GetCurrentProcess(), 1)` at `0x6154F0`

---

## 5. Layout

```
decomp/          readable decompiled C/C++  <- the main product
runtime/         hand-written native PC layer
  platform/      window, input (XInput/DInput), timing, files, mod VFS
  stubs/         removed code (SecuROM, affinity lock, etc.)
tests/           decomp vs original comparison
tools/           asset extractors and utilities
OUTPUT/          analysis and dumps (not versioned except OUTPUT/tools)
docs/            formats, limits, architecture notes
```

---

## 6. Phases

See the table in `PROGRESS.md §1`.
