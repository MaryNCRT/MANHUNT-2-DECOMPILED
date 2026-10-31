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

Planned harness (`tests/`, **not built yet**):
1. A 32-bit test program maps `Manhunt2_nosecu.exe` at its preferred base `0x400000`
   (no relocations needed), resolves its imports, and does **not** run its entry point.
2. For a leaf function (one that calls nothing, or only CRT), the test calls the original
   at its address and our rewrite with the same generated inputs, and compares outputs and
   memory side effects.
3. For functions that call others, those callees are either already verified or replaced
   by recording stubs, so only one function is under test at a time.
4. Acceptance: **zero divergences** across thousands of inputs, including edge cases.

Where running the code is impractical (rendering, Windows calls, global state that only
exists mid-game), the fallback is a static comparison: constants, call order, and branch
conditions of our code against the disassembly, plus behavior checks once the port runs.

Until this harness exists, **the verified count stays at 0 %**, and that is the honest number.

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
