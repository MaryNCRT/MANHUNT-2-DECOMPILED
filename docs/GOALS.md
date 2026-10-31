# Goals

## Must have
1. **Readable source code** rebuilt from the 2009 PC executable.
2. **Native 32-bit and 64-bit** Windows builds from the same source.
3. **No 2 GB memory limit.**
4. **No fixed limits** on textures, models, pools, entities or buffers.
5. **Original mods keep working**: every original file format read exactly as before;
   every known ASI plugin's feature available natively. See [MODS.md](MODS.md).
6. **HD models and textures** load without hacks.
7. **Runs on Windows 10 and 11** from the first build, with no third-party patches
   (single-core lock, DEP, input, timing).
8. **Set up from the user's own copy**: an installed game folder or the disc ISO.
9. **Zero game files in the repo.**

## Later
- Correct widescreen and FOV
- Unlocked framerate with fixed-step game logic
- XInput controllers, raw mouse input
- Native plugin API for new mods
- Modern renderer (D3D11/Vulkan) after the D3D9 path is faithful

## Not goals
- Removing or bypassing DRM on the retail executable
- Loading old ASI plugins by address (impossible on a rebuilt exe; see [MODS.md](MODS.md) §3)
- Changing gameplay or content of the original game
