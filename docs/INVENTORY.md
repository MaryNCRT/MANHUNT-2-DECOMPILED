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

## What was left out: 1,765 BinViz-only functions (~70 KB)

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

Game-code functions went from 14,929 to 15,677. The treemap's game total (BinViz's view)
stays larger by the 1,765 left-out functions; the README percentages use our list.
