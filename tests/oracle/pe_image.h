// pe_image.h — maps the original Manhunt2.exe into the test process so its
// functions can be called directly. See docs/METHODOLOGY.md, "The oracle".
//
// The image is mapped at its preferred base 0x00400000 (it has no relocation
// table), sections are copied, BSS is zeroed, and imports are bound. The
// original entry point never runs, so none of the game's start-up happens.
//
// Imports are bound to the real DLL only for an allow-list of system DLLs.
// Everything else (d3dx9, FMOD, Bink, DirectInput...) is bound to a trap that
// reports which import was called and stops the test: a function under test
// must not reach those.
#pragma once

#include <windows.h>
#include <cstdint>

namespace oracle {

constexpr uintptr_t kImageBase = 0x00400000;

// Expected SHA-1 of the reference executable (PROGRESS.md §2).
constexpr const char* kReferenceSha1 = "9a7053ada233b3aeb2433509dbeba9c45b086cbc";

// Maps `exePath`. The file is read and hash-checked first; then
// `releaseRange` is called to free 0x00400000 (the host image), and the range
// is reserved immediately, before anything else can allocate into it.
// Returns false and prints the reason on failure.
bool MapOriginal(const wchar_t* exePath, bool (*releaseRange)());

// Address of something inside the mapped original.
template <typename T>
T At(uintptr_t va) { return reinterpret_cast<T>(va); }

// Copies of the original's writable sections as they were right after mapping,
// so tests can reset global state between cases.
void SnapshotData();
void RestoreData();

}  // namespace oracle
