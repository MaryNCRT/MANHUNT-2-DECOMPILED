# Contributing

New here? Read [GETTING-STARTED.md](GETTING-STARTED.md) and
[METHODOLOGY.md](METHODOLOGY.md) first.

## The hard rules

1. **Never commit anything from the game.** No exe, DLL, asset, extracted file, or
   long dumps of decompiler output. `.gitignore` blocks the usual suspects; check
   before you add something new. Addresses, sizes and our own names are fine.
2. **Never break, document or ship DRM removal.**
3. **No code copied from mods or community tools.** Read them to learn; write your own.
   GPL code must never be pasted in.
4. **Mark what you are not sure of.** `(?)` after a name, `UNRESOLVED:` before a
   comment. A wrong name stated with confidence costs more than an honest unknown.
5. **Every change must build for x86 and x64** (once the build exists). No pointers in
   `int`/`DWORD`.
6. **Original mods must keep working.** A change that makes the port read an original
   file differently is a bug. See [MODS.md](MODS.md).

## Picking up work

- `python tools/tracking.py report` shows where things stand.
- [PROGRESS.md](../PROGRESS.md) §"Next work" lists what is being worked on now.
- Good first targets: small functions (under 100 bytes) whose callers are already
  readable. `decomp/functions.tsv` has sizes; `CallTree.java` shows the neighbours.

## Workflow for one function

```powershell
# 1. Draft
"0x00615420" | Set-Content OUTPUT\list.txt
.\OUTPUT\tools\ghidra.ps1 DecompileList.java OUTPUT\list.txt OUTPUT\draft.c

# 2. Read the disassembly where the draft is doubtful (any disassembler works;
#    Ghidra's GUI, x64dbg, or capstone)

# 3. Write the clean version in decomp/<module>/<File>.cpp

# 4. Record it
python tools\setstatus.py 615420 status=readable name=CountProcessesNamed file=decomp/core/WinMain.cpp

# 5. Refresh the numbers in README/PROGRESS
python tools\tracking.py report
```

## Commit conventions

- English, imperative subject line: `Decompile WinMain and process check`.
- Say in the body what was verified and how, and what is still uncertain.
- If an AI wrote the change, keep the `Co-Authored-By:` trailer. See
  [AI-DISCLOSURE.md](AI-DISCLOSURE.md).
- Run `python tools/tracking.py report` before committing so the README matches.

## Reporting findings without code

Open an issue with: the address, what you think the function or variable is, and the
evidence (string, API call, behavior seen in game, plugin that patches it). That is
often worth more than code.
