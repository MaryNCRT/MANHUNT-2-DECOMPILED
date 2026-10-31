# AI Disclosure

You have a right to know how the code in this repository was produced. AI-assisted
reverse engineering is controversial, so we state it plainly.

## The short version

**Most of this project is produced by Anthropic's Claude, working through Claude
Code, under the direction of MaryNCRT.** Claude appears in the commit history as a
co-author through the standard `Co-Authored-By:` trailer.

## Who does what

| Part | Who |
|---|---|
| Project direction, goals, scope, priorities | MaryNCRT |
| Analysis tooling (`peinfo.py`, Ghidra scripts, tracking) | AI |
| Decompiled code in `decomp/` | AI, checked against the disassembly |
| Documentation | AI, reviewed by MaryNCRT |
| Whether any of it is *correct* | **Tests against the original binary**, not opinions |

## Why this can still be trusted

A language model writes plausible code whether or not it is right. The project is
built around that fact:

- Every function is checked against the **disassembly**, not just the decompiler draft.
  Example: Ghidra dropped the return value of `0x00615420`; the disassembly showed it.
- Uncertainty is written down (`(?)`, `UNRESOLVED:`) instead of hidden.
- The README separates **readable** from **verified**. Only functions that pass a
  differential test against the original code (see [METHODOLOGY.md](METHODOLOGY.md))
  count as verified. Until that harness exists the verified number is 0 %, and the
  README says so.
- Progress numbers are computed by a script, not written by hand.

## If you would rather not use AI-assisted code

That is a legitimate position. The tracking file and documentation say which code
exists and how it was checked; human-written reimplementations are welcome.
