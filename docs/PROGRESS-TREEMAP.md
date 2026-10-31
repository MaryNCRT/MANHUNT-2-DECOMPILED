# Decompilation treemap (BinViz)

`docs/progress/decompilation.svg` is a byte-weighted treemap of the executable:
one tile per function, its area proportional to the function's code bytes, its
color its status. It is drawn by [BinViz](https://github.com/trevor-e/binviz)
from our own tracking data.

**`decomp/functions.tsv` stays the only source of truth.** BinViz is only the
drawing layer: `tools/generate_treemap.py` translates the TSV into a BinViz notes
file and asks BinViz to render it. The percentages in the README tables still
come from `tools/tracking.py`, unchanged.

## What the colors mean here

BinViz was made for *matching* decompilations, so its states have precise
meanings. This is how ours map onto them (`tools/generate_treemap.py`):

| Ours | BinViz state (legend) | Meaning in BinViz |
|---|---|---|
| `verified` | **Nonmatching C** | C that does the same as the original but does not compile to the same bytes: exactly what the oracle proves |
| `readable` | **In progress** | being worked on; our C exists but its behavior is not proven |
| `todo` | **To do** | not started |
| `dropped` | **Skipped** | removed on purpose |
| `replaced`, and every CRT, import, exception funclet and DXUT function | **Library** | linked, not decompiled; outside game totals |

**Nothing is ever "Matched".** In BinViz that means the C compiles to the very same
bytes, which this project does not attempt (it targets modern MSVC, x86 and x64).
BinViz's headline "% matched by bytes", and any "matching" badge, therefore read
0 % by design. That number is not our progress; our progress is the
*readable* and *verified* rows of the README table.

Other fields: `reviewed` is set only for functions read against the disassembly
(readable/verified); `comment` records the original category and status, any
tentative `(?)` name, and the full Ghidra size when it was cut (below); the source
file of decompiled functions groups the treemap.

### Extents

Ghidra's function sizes include non-contiguous pieces, while a BinViz note is one
contiguous block. Where a function would overlap the next one (148 functions,
43,549 bytes at the time of writing), the note is cut at the next function's start
and the full size is kept in the comment. The script prints how many were cut.

## Installation

1. Rust 1.88 or newer (`winget install Rustlang.Rustup`, then
   `rustup default stable-x86_64-pc-windows-msvc`).
2. Build the BinViz CLI into `TOOLS/binviz` (not versioned):
   ```powershell
   git clone --depth 1 https://github.com/trevor-e/binviz REFERENCE\binviz
   cd REFERENCE\binviz
   cargo build --release --locked -p binviz-cli --target-dir "..\..\TOOLS\binviz"
   ```
   The script finds `TOOLS/binviz/release/binviz.exe`, `binviz` on PATH, or the
   `BINVIZ` environment variable.
3. The reference executable at `ORIGINAL/Manhunt2_nosecu.exe` (see GETTING-STARTED).

## Usage

```powershell
python tools/generate_treemap.py --report   # tables, bars and progress map, then the treemap
python tools/generate_treemap.py            # treemap only
python tools/tracking.py report             # tables only, as before
```

The script validates the TSV first (columns, hex addresses, positive sizes,
known categories and statuses, no duplicates, same function set as the Ghidra
inventory when present) and stops on any error. After rendering it reads
BinViz's totals back from the SVG, fails if BinViz reports any matched code, and
prints both BinViz's and our numbers.

## Why there is no GitHub Action

Rendering needs the original executable, which never enters the repository, and
a Rust build of BinViz. So the treemap is regenerated locally and committed with
the change that moved the numbers.
