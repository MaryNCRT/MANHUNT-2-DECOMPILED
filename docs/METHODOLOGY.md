# Methodology

How the decompilation is done, and why it is done this way. Same method as the
UMK3 iOS project, adapted to an x86 Windows binary with no symbols.

## The problem with decompilers

Ghidra's output looks like C, but it is a guess. It loses return values (in
`0x00615420` it dropped the process counter entirely), invents or forgets
arguments, mistypes floats as integers, and cannot see through C++ virtual calls.
An AI rewriting that draft will produce code that *looks* right with the same
confidence whether it is right or not.

So the project assumes **neither the decompiler nor the person/AI writing the
clean code can be trusted**, and correctness must come from somewhere else.

## Two levels of "done"

| Status | Meaning | Counted as |
|---|---|---|
| `readable` | Clean C/C++ in `decomp/`, names and types worked out, every uncertainty marked `(?)` or `UNRESOLVED`. Checked by reading the **disassembly**, not just Ghidra's draft. | "decompiled" |
| `verified` | Same as readable, **plus** proven to behave like the original by a test against an oracle. | "verified" |

The README shows both numbers separately. A readable function can still be wrong;
only verified functions are trusted.

## The oracle: run the original function

UMK3 had to recompile ARM code into C to get a reference. Manhunt 2 is easier:
the reference exe is a normal 32-bit Windows program, so **the original code can
run directly on the same machine**.

The harness is in `tests/oracle/` and runs with `.\tests\oracle\build.ps1` (32-bit MSVC).

1. **Mapping.** The original must sit at `0x00400000` (it has no relocation table), but
   Windows puts the process heaps and NLS views in that range before any of our code runs.
   So `oracle_host.exe` is a CRT-free placeholder linked *at* `0x00400000` with an image
   big enough to cover the original's range. It loads `oracle.dll`, which reads and
   hash-checks `Manhunt2_nosecu.exe`, unmaps the host image, immediately reserves the
   range, copies the sections and binds the imports. The original's entry point never runs.
2. **Imports.** System DLLs (kernel32, user32, …) are bound for real. Everything else
   (d3dx9, FMOD, Bink, DirectInput) is bound to a trap that names the import and stops
   the run, so a test can never silently call into something we do not provide.
3. **One test** calls the original at its address and our rewrite with the same inputs and
   compares the results. Writable sections are restored between tests.
4. **Live inputs** (process list, clock): the original runs before and after ours; if its
   two results differ, the system changed mid-case and the case is skipped and reported as
   *unstable*, never counted as a pass.
5. **Acceptance: zero divergences** across all cases, including edge cases.

Not built yet: stubs for callees (so a function that calls others can be tested alone),
`__thiscall` call helpers, and comparison of memory side effects. Until those exist,
only leaf functions and functions whose callees are system DLLs can be verified.

Where running the code is impractical (rendering, global state that only exists mid-game),
the fallback is a static comparison: constants, call order and branch conditions of our
code against the disassembly, plus behavior checks once the port runs.

### Results so far

| Function | Cases | Divergences |
|---|---|---|
| `AlignUp64` 0x0060DA60 | 1,131,072 (all of 0–0xFFFF, all of 0xFFFF0000–0xFFFFFFFF, 1 M random) | 0 |
| `CountProcessesNamed` 0x00615420 | 220 per run, 11 names × 20 rounds (live process list) | 0 |
| `GameApp::SetDefaults` 0x004DF5C0 | 156,000 (2,000 objects × every byte, random default heights) | 0 |
| `GameApp::HandleEvent` 0x004DF660 | 376,900 (events −8…0x40 × 50 + 100,000 random), **except events 0x15 and 0x16** | 0 |
| `SetVersionString` 0x00424990 | 240,000 (20,000 strings of 0–20 chars × 12 buffer bytes) | 0 |
| `AlwaysTrue` 0x004202E6 | 1,000 | 0 |
| `SetLoadingMessage` 0x004B0570 | 100,000 | 0 |
| `Vec4Table_F620::Construct` 0x004B08C0 | 1,260,180 (20 random-filled objects × every byte, plus the return value) | 0 |
| `ResetGlobals_0076BD80` 0x0056EFB0 | 113,000 (fields, plus every neighbouring byte left untouched) | 0 |
| `File::ReadLine` 0x0053ABE0 | 13,200,000 (200,000 random streams with `\r`, `\n`, `\0`; max −2…21; return, 64-byte buffer and bytes consumed) | 0 |
| `PackedFile::IsEOF` / `GetSize` 0x0053ACF0 / 0x0053AD10 | 400,000 (random, at-end and one-before-end positions) | 0 |

`File::ReadLine` calls virtual methods. The original is given a fake object whose
vtable slots point at test functions serving the same byte stream as our side, so both
read identical input and their consumption is compared.

### Reading functions SecuROM split

Some functions were cut into pieces joined by `jmp`s, so a listing by size stops halfway
and Ghidra's draft can skip calls. `python OUTPUT/tools/disasm.py <exe> <addr> follow`
reads in execution order across the pieces; `CApp_Init` was decompiled that way, and it
showed two calls Ghidra's draft had wrong.

`HandleEvent` stays *readable*, not *verified*, until events 0x15 (`Initialise`) and
0x16 (`Shutdown`) can be tested too: their callees are not decompiled yet, and in the
test build they are stubs that stop the run if reached.

`__thiscall` functions of the original are called through a `__fastcall` pointer:
`ECX` carries the object, `EDX` is a dummy, and the callee cleans the stack the same way.

The first run of `CountProcessesNamed` reported 1 divergence (375 vs 376 matches for
`"e"`): a process started between the two calls. That was a flaw in the test, not in the
function, and is why live inputs now use the before/after rule above.

## Step by step, for one function

1. **Pick it.** Start from what calls it (`CallTree.java`) and what it references (strings,
   globals). Small leaf functions first; they unlock their callers.
2. **Draft.** `DecompileList.java` gives Ghidra's C.
3. **Read the disassembly** wherever the draft looks odd: return values, argument counts,
   calling convention (`__thiscall` passes `this` in `ECX`), floats.
4. **Name it** from evidence: strings it uses, the API it calls, the DXUT/CRT routine it matches.
   No evidence, no name: keep `Unknown_<addr>`.
5. **Write the clean version** in `decomp/<module>/`, with a header comment listing each
   function, its address, its size and its status.
6. **Record it**: `python tools/setstatus.py <addr> status=readable name=... file=...`.
7. **Verify** once the harness exists, and set `status=verified`.

Every intentional change from the original is marked `PORT:` in the code and explained.

## What is not decompiled

| Code | Why | What happens instead |
|---|---|---|
| MSVC CRT (identified by Ghidra FID) | Our compiler ships its own | Linked normally |
| DXUT | Public Microsoft sample code, tied to 2009 Windows | Replaced by our own platform layer in `runtime/` |
| SecuROM leftovers | DRM | Dropped |
| Import thunks | Just jumps into DLLs | Real calls |

## 32-bit and 64-bit from the same source

The original stores pointers in 32-bit fields everywhere. Decompiled code must not:
pointers stay pointers, sizes use `size_t`, and structures read from files use
fixed-size types with `static_assert` on their size. File parsing never relies on the
in-memory layout of a struct that contains pointers. This is what makes the x64
build possible without breaking original data files and mods.

## Honest numbers

Percentages are **measured, by bytes of game code**, by `tools/tracking.py` from
`decomp/functions.tsv`. CRT, DXUT, import thunks and exception funclets are
excluded from the total, because they are not decompiled. Nobody types a percentage by hand.
Phase estimates in PROGRESS.md that cannot be measured are labelled as estimates.
