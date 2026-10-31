# Manhunt 2 — PC Decompilation

**English** · [Español](README_es.md)

A decompilation of the PC executable of **Manhunt 2** (2009) into readable source
code, to build a native **32-bit and 64-bit** port for Windows 10/11:

- no 2 GB memory limit
- no texture, model, pool or buffer limits
- HD model and texture support, mods loaded from folders
- **original game mods keep working** ([how](docs/MODS.md))
- no third-party patches needed to run on modern systems

## Progress

**Status:** decompilation has started. Nothing compiles yet. Nothing is playable.

<!-- progress:start -->
<!-- progress:end -->

These numbers are computed from [`decomp/functions.tsv`](decomp/functions.tsv) by
[`tools/tracking.py`](tools/tracking.py); nobody types them by hand. "Decompiled"
and "verified" are different things: see [METHODOLOGY.md](docs/METHODOLOGY.md).
Per-phase status, including the estimates we cannot measure, is in [PROGRESS.md](PROGRESS.md).

## Documentation

| Read this | If you want to |
|---|---|
| [GETTING-STARTED.md](docs/GETTING-STARTED.md) | understand the project from zero, with vocabulary and setup |
| [METHODOLOGY.md](docs/METHODOLOGY.md) | know how functions are decompiled and checked |
| [CONTRIBUTING.md](docs/CONTRIBUTING.md) | help: rules, workflow, commit conventions |
| [GOALS.md](docs/GOALS.md) | see what the port must do, and what it will not |
| [MODS.md](docs/MODS.md) | know how original mods and ASI plugins are supported |
| [AI-DISCLOSURE.md](docs/AI-DISCLOSURE.md) | know how the code is produced |
| [PROGRESS.md](PROGRESS.md) | see the full project state and verified facts |

## How the port will be set up

You will need your own copy of Manhunt 2 for PC, either:

- an **installed game folder**, or
- the **game disc ISO**.

The port reads the game data from either one. It never uses the original
executable.

## Legal

This repository contains **only original code and documentation**. It does not
include the executable, DLLs or any game asset. The project does not include or
document DRM removal.

Manhunt 2 is the property of Rockstar Games / Take-Two Interactive. This project is
not affiliated with them.
