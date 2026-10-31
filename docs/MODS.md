# Mod support

**Hard requirement:** mods made for the original PC game must keep working in the
port. This document says what that means for each kind of mod, and is honest
about the one kind that cannot work as-is.

## 1. Kinds of original mods

| Kind | How it works in the original | Port support |
|---|---|---|
| **Data mods** — replaced models, textures, animations, audio banks, level scripts, text | Files overwritten inside the game folder | **Full.** The port reads every original file format byte-for-byte. Raising a limit never changes how an original file is parsed. |
| **Mod-loader folders** (e.g. `Manhunt2.ModLoader.asi`) | A plugin redirects file reads to a mod folder | **Full, natively.** The port has a virtual file system with mod folders (§2). The layouts used by existing loaders are read too, so those mods install unchanged. |
| **Tool-made content** (Manhunt Toolkit, BLeeds, Storyland, IFP scripts…) | Tools write original-format files | **Full**: it is still data in the original formats. |
| **ASI plugins** (`.asi` DLLs via Ultimate ASI Loader) | The DLL patches code and data **at fixed addresses inside Manhunt2.exe** | **Cannot load as-is** (§3). Their features are rebuilt natively, and the port offers its own plugin API. |

## 2. Mod folders (data mods)

- Original game data is never modified.
- `mods/<mod name>/` mirrors the game tree (`global/`, `levels/`, `audio/`…).
  A file there replaces the original file with the same path.
- Load order is a plain text list (`mods/load_order.txt`); later mods win.
- Files may be larger or have more entries than the original allowed. See
  `docs/LIMITS.md` (to be written) for every limit the original had and the
  port's replacement.
- Mods that only exist as "overwrite the game folder" instructions also work:
  the port reads the installed folder as the base layer.

## 3. ASI plugins: why they cannot load as-is

An ASI plugin is a DLL that writes new bytes at hard-coded addresses of the
2009 executable, for example `0x6154F0` for the single-core affinity lock. The
port is a newly compiled program. Its functions and data live at different
addresses, and in the 64-bit build they are even a different instruction set.
A plugin that patches `0x6154F0` would overwrite unrelated code.

There is no honest way to promise binary compatibility. What the port does instead:

1. **Every known plugin's feature becomes a built-in option**, usually as a
   setting that is on by default when the plugin fixed a bug. Catalog below.
2. **A native plugin API** (C ABI, same for x86 and x64) for new mods: hooks
   for update, render, input, file loading, script functions and the debug menu.
   This replaces address patching with stable entry points.
3. Plugins that only call Windows or D3D9 APIs and never touch the exe (for example
   ReShade-style injectors) keep working, because the port still uses D3D9 on the first renderer.

### Known plugins → native feature

Collected from the plugins found in existing installs. Status is updated as
each one is done.

| Plugin | What it does (per name/readme, to be verified) | Port plan | Status |
|---|---|---|---|
| `mh2_affinity.asi` | Removes forced single-core affinity | Call removed (see `decomp/core/WinMain.cpp`) | 🔄 located at `0x6154F0` |
| `Manhunt2.NoTexureLimit.asi` | Raises the texture limit | Limit removed at the source | ⬜ |
| `BloodDecalFix` / `BloodDecalFixV2` | Blood decal bug fix | Fixed in decompiled code | ⬜ |
| `Mh2DisableQTM` | Disables QTM (to be identified) | Option | ⬜ |
| `Manhunt2.Keyboard.asi` | Keyboard/mouse improvements | Built-in input layer | ⬜ |
| `Manhunt2.XInput.asi` / `HybridInput` | XInput controllers | Built-in XInput | ⬜ |
| `AbsoluteCamera.asi` | Camera control change | Option | ⬜ |
| `Manhunt2.DebugMenu.asi` | Debug menu | Built-in debug menu | ⬜ |
| `Mh2Modern.asi` | Modern-system fixes (to be identified) | Each fix built in | ⬜ |
| `Manhunt2.UM2.asi` | Mod bundle features (to be identified) | Reviewed one by one | ⬜ |
| `Manhunt2.XboxRainDroplets.asi` | Screen rain droplets effect | Renderer feature | ⬜ |
| `Manhunt2.CoronaPickups.asi` | Pickup highlight coronas | Renderer feature | ⬜ |
| `Manhunt2.Achievements.asi` | Achievements | Via plugin API | ⬜ |
| `ManhuntCrashAnalyzer.asi` / `GenericUnhandledException.asi` | Crash reports | Built-in crash handler | ⬜ |
| `DiscordPlugin.asi` | Discord presence | Via plugin API | ⬜ |
| `_noDEP.asi` | Works around DEP crashes | Not needed: the port is built DEP-compatible | ⬜ |
| `Manhunt2.ModLoader.asi` | Mod file loader | Mod folders (§2), same layout accepted | ⬜ |
| `PluginMH2.asi` | To be identified | Reviewed | ⬜ |

Plugin binaries are only inspected to learn which limit or bug they address;
their code is not copied.
