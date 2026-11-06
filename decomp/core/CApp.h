// CApp.h — CApp_Init and the small pieces it starts that are decompiled.
#pragma once

#include <cstdint>

// 0x007D27F0 (via thunk 0x004B12D0) — logs "Initialising CApp" and starts the
// game's core systems. Fails only if the 0x005898C0 step fails.
bool CApp_Init();

// 0x004B0570 — remembers the message to show while loading (0x006ECBA4).
void SetLoadingMessage(const char* message);
extern const char* g_LoadingMessage;

// The 0xF620-byte object created by CApp_Init (singleton at 0x00758690).
// Two tables of 4-float records, zeroed by the constructor; what the records
// mean is not known yet.
struct Vec4Table_F620 {
    float   a[0xA8C][4];  // +0x0000
    float   b[0x384][4];  // +0xA8C0
    uint8_t unkE100[0xF618 - 0xE100];
    int32_t unkF618;      // +0xF618
    int32_t unkF61C;      // +0xF61C

    // 0x004B08C0 — __thiscall; returns this.
    Vec4Table_F620* Construct();
};
static_assert(sizeof(Vec4Table_F620) == 0xF620, "Vec4Table_F620 size");

// 0x0056EFB0 — zeroes the block below. Its users are not known yet.
struct Globals_0076BD80 {
    int32_t words[36];  // 0x0076BD80..0x0076BE0C
    int32_t at0076BE40;
    uint8_t at0076BE50;  // a byte in the original, not a dword
};
extern Globals_0076BD80 g_Globals_0076BD80;
void ResetGlobals_0076BD80();
