# Getting Started

This page assumes you know nothing about the project, and little about reverse
engineering. Read it top to bottom once; after that, [CONTRIBUTING.md](CONTRIBUTING.md)
is the page you will come back to.

## 1. The situation, in plain terms

Manhunt 2 came out on PC in 2009 as one program, `Manhunt2.exe`, plus game data
(models, textures, sounds, level scripts). Rockstar never released the source code.

The program still works, but it is stuck in 2009. It can only use 2 GB of memory,
it has fixed-size tables that mods keep hitting (textures, models…), it pins
itself to a single CPU core, and it only knows DirectInput and Direct3D 9.
Modders work around this with plugins that patch bytes inside the exe at fixed addresses.

This project rebuilds the **source code** of the program by reading its machine
code, function by function, and rewriting each one as readable C/C++. When
enough of it is rebuilt, we compile our own `Manhunt2.exe`: for 32-bit and
64-bit Windows, with the limits removed, still reading the original game data.

## 2. Vocabulary

| Word | Meaning |
|---|---|
| **Binary / executable / exe** | `Manhunt2.exe`: the program as machine code, the only thing we have. |
| **Function** | One routine of the program. The exe has ~15,400 of them. |
| **Address** | Where a function or variable lives inside the exe, e.g. `0x006155C0`. It is the function's ID until we know its name. |
| **Disassembly** | Machine code printed as assembly instructions. Exact, but hard to read. |
| **Decompiler** | A tool (we use **Ghidra**) that turns machine code into C-like code. Readable, but often **wrong in the details**. |
| **Decompiling** (in this project) | Taking Ghidra's draft, understanding it, and writing correct, readable C/C++ with real names and types. |
| **Symbols** | Function and variable names. **This exe has none**: every name in the project was worked out by hand. |
| **CRT** | The C runtime library compiled into the exe (`malloc`, `fopen`…). We do not decompile it; our compiler provides it. |
| **DXUT** | The DirectX SDK sample framework the game used for its window and device. We replace it instead of decompiling it. |
| **SecuROM / `.secu`** | The DRM on the retail exe. We do not touch it. Note that SecuROM moved ~1,000 real game functions into a section called `.secu`, and those are game code we do decompile. |
| **Oracle** | A trusted reference to compare our code with. See [METHODOLOGY.md](METHODOLOGY.md). |
| **ASI plugin** | A mod DLL that patches the original exe at fixed addresses. See [MODS.md](MODS.md). |

## 3. The pieces

### The binary
You need your own copy of the game. The retail exe is SecuROM-encrypted and
cannot be read; analysis uses the unprotected 2009 exe that circulates in the
modding scene, identified by its hash in [PROGRESS.md](../PROGRESS.md) §2. The
project does not explain or provide DRM removal, and **never commits the exe or
anything extracted from it**.

### Ghidra
NSA's open-source decompiler. We run it "headless" (from scripts, without its
window) so every result can be reproduced by anyone. Our scripts live in
`OUTPUT/tools/*.java`.

### REA
An MCP server and CLI that lets an AI agent query Ghidra: function dossiers,
call graphs, value tracing. Optional for humans, used by agents.

### The tracking file
`decomp/functions.tsv` lists **every function** of the exe with its size, a category
(game, DXUT, CRT…) and a status (todo, readable, verified…). It only holds addresses,
sizes and our names, no code from the game. The progress numbers in the README are computed from this
file by `tools/tracking.py`, never written by hand.

### decomp/
The product: readable C/C++ rebuilt from the binary, one file per module.
`decomp/core/original.h` declares the functions that are referenced but not
decompiled yet, each with its address.

## 4. Setting up

### What you need
- Windows 10/11 x64
- Python 3.12+ with `pefile` and `capstone` (`pip install pefile capstone`)
- Ghidra 12.1.x and a 64-bit JDK 21+
- Your copy of Manhunt 2 PC
- Optional: Node.js 22+ and REA (`npm install rea-agents@6.3.0`)

### First steps
```powershell
# 1. Put the unprotected exe at ORIGINAL\Manhunt2_nosecu.exe and check its hash
certutil -hashfile ORIGINAL\Manhunt2_nosecu.exe SHA1
#    expected: 9a7053ada233b3aeb2433509dbeba9c45b086cbc

# 2. Import and analyze it in Ghidra (takes a while), dump the inventory
$env:JAVA_HOME = "C:\path\to\jdk-21"
TOOLS\ghidra_12.1.2_PUBLIC\support\analyzeHeadless.bat OUTPUT\ghidra MH2 `
    -import ORIGINAL\Manhunt2_nosecu.exe -scriptPath OUTPUT\tools `
    -postScript ExportInventory.java OUTPUT\inventory

# 3. Decompile a function to see Ghidra's draft
"0x6155c0" | Set-Content OUTPUT\list.txt
.\OUTPUT\tools\ghidra.ps1 DecompileList.java OUTPUT\list.txt OUTPUT\draft.c

# 4. See its callers' tree
.\OUTPUT\tools\ghidra.ps1 CallTree.java 0x6155c0 2 OUTPUT\tree.txt

# 5. Current progress
python tools\tracking.py report
```

Then compare `OUTPUT\draft.c` with `decomp/core/WinMain.cpp` to see what
"decompiling" means here: same logic, real names, mistakes fixed, uncertainties marked.

## 5. Where you can help without writing code

- **Identify functions**: if you recognize what a function does (from strings,
  from modding knowledge), add its name with `tools/setstatus.py` and a source.
- **File formats**: the community already documented many (MDL, TEX, FSB…).
  Linking a format to the function that reads it is very valuable.
- **Limits**: every known mod-breaking limit, with how to reproduce it, goes in `docs/LIMITS.md`.
- **Plugins**: what each ASI plugin in [MODS.md](MODS.md) actually fixes.
