# Function inventory

How the list of functions in `decomp/functions.tsv` was built, and how complete it is.
Every progress percentage is measured against this list, so its gaps matter.

## Sources

1. **Ghidra auto-analysis** of the reference exe: 15,407 functions
   (`OUTPUT/tools/ExportInventory.java`).
2. **Cross-check with BinViz**, which finds functions with its own analysis
   (`OUTPUT/tools/binviz_crosscheck.py`). On 2026-10-31 BinViz saw 2,512 functions
   (90,865 bytes) that Ghidra had not created. None overlapped a Ghidra function;
   Ghidra had none that BinViz lacked.

## What was added: 748 functions

A BinViz-only function was added only with strong evidence that something uses it,
**and** only if its code ends like a function (`ret`, or a `jmp` for thunks and tail calls):

| Evidence | Functions |
|---|---|
| Address in a function-pointer table in `.rdata`/`.data` (vtables, callbacks, static initializers), next to another function address | 678 |
| Target of a direct `call` | 47 |
| Address taken in code (`push`/`mov`/`lea`) | 15 |
| Created by Ghidra while following the code of the above (thunk targets, a 2-byte thunk split from its function) | 8 |

They were created in the Ghidra project with `OUTPUT/tools/CreateFunctions.java`, which
writes the evidence as each function's plate comment, and the inventory was exported again.

Hand checks before adding, in the disassembly: a `__thiscall` method called through a
vtable (0x490F50), a static-initializer stub `push; call _atexit` (0x632B50), a loop
method (0x550A30) and a thunk called directly (0x468A40) were all real functions.

**Rejected rule:** "target of a `jmp`, ending in `ret`" first looked like evidence of a
tail-called function, but the sample 0x53DED5 is a single `ret` other code jumps to: a
shared function tail, not a function. That rule (76 candidates) was dropped.

## zlib (2026-11-05)

zlib 1.1.4 is linked statically (version string "1.1.4" passed to `inflateInit_` by
`File_Open`). Eight functions are categorized `zlib` and count as library, outside the
game totals: the six that use zlib's own error strings, `inflateInit_` (0x006117A0) and
`inflateEnd` (0x00611650, called by the packed reader's `Close`). Their neighbours in
0x0060FF00–0x00612800 are probably zlib too (the copyright strings of both inflate and
deflate are present), but without evidence they still count as game code: the game
total is slightly too large rather than too small.

## Second round: 560 more (2026-11-05)

The first round's "direct call" and "address taken" scans used capstone's linear
disassembler, which **stops at the first byte it cannot decode**: on `.text` it covered
0.3 % of the section. With `skipdata` it covers all of it. To keep a sweep that lost
sync after embedded data from inventing references, a call or a taken address now only
counts if its instruction lies inside a function Ghidra already knows.

Result: **560 functions whose address is pushed or loaded inside a known function**
(callbacks, and the destructors that static initializers register with `_atexit`).
560 of the 563 candidates passed the in-function rule; 3 were dropped. Ghidra created
5 more while following them, and one existing function shrank from 6,534 to 12 bytes:
Ghidra had merged unreferenced code that now has functions of its own.

## What was left out: 1,205 BinViz-only functions

After the second round:

| Shape | Count |
|---|---|
| Code that does not end in `ret` or `jmp` (fragments) | 444 |
| Lone `jmp`, no reference | 399 |
| Ends in `jmp`, no reference | 157 |
| Ends in `ret`, no reference | 141 |
| Up to three instructions ending in `ret`, no reference | 64 |

## What was left out in the first round: 1,765 BinViz-only functions (~70 KB)

| Shape | Count |
|---|---|
| Code that does not end in `ret` or `jmp` (fragments) | 438 |
| Ends in `jmp`, no reference (tails, fragments) | 430 |
| Lone `jmp`, no reference | 405 |
| Ends in `ret`, no reference | 255 |
| Up to three instructions ending in `ret`, no reference | 231 |
| Bytes that do not decode as code | 6 |

Some of these may be real: dead code, or functions reached only through computed
addresses. They are not counted until something shows they are. Re-run the cross-check
after any large change to the Ghidra project:

```powershell
TOOLS\binviz\release\binviz.exe progress OUTPUT\binviz\Manhunt2.exe `
    --notes OUTPUT\binviz\Manhunt2_nosecu.exe.binviz-notes.json json > OUTPUT\binviz\progress.json
python OUTPUT\tools\binviz_crosscheck.py          # writes OUTPUT\binviz\create_functions.txt
# then, if it kept anything:
TOOLS\ghidra_12.1.2_PUBLIC\support\analyzeHeadless.bat OUTPUT\ghidra MH2 -process Manhunt2_nosecu.exe `
    -noanalysis -scriptPath OUTPUT\tools -postScript CreateFunctions.java OUTPUT\binviz\create_functions.txt `
    -postScript ExportInventory.java OUTPUT\inventory
python tools\tracking.py init
```

## Effect on the numbers

Game-code functions went from 14,929 to 15,677 (first round) and then to 16,242 (second round). The treemap's game total (BinViz's view)
stays larger by the left-out functions; the README percentages use our list.
