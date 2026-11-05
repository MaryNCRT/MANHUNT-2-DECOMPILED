// GameApp.h — the main game object (static at 0x0079E728, g_Game points to it)
// and its event handler.
//
// The handler returns 0, 1 or 2 and the object starts with an application
// name, a resolution and a frame limit. That is the shape of an application
// skeleton's event handler (error / processed / not processed); RenderWare's
// skeleton is the best-known one (?). Event numbers are named only where the
// code shows what they do.
#pragma once

#include <cstddef>
#include <cstdint>

enum GameEventStatus : int {
    kEventError        = 0,
    kEventProcessed    = 1,
    kEventNotProcessed = 2,
};

enum GameEvent : int {
    kEvent_ReturnsItself1  = 0x01,  // answered with its own number
    kEvent_04              = 0x04,  // processed, nothing done
    kEvent_0A              = 0x0A,  // AlwaysTrue()
    kEvent_Initialise      = 0x15,  // GameApp::Initialise(param)
    kEvent_Shutdown        = 0x16,  // 0x004DF4A0
    kEvent_17              = 0x17,  // AlwaysTrue()
    kEvent_SetDefaults     = 0x18,  // GameApp::SetDefaults()
    kEvent_19              = 0x19,  // processed, nothing done
    kEvent_SetsFlag1C      = 0x1E,  // sets the +0x1C flag, then not processed
    kEvent_ReturnsItself25 = 0x25,
    kEvent_ReturnsItself26 = 0x26,
};

struct GameSubObject;  // 0x0079E70C, reached through +0x0C; not decompiled yet

// Layout as observed; offsets are the original's 32-bit ones and are only
// asserted on x86. Fields with unknown meaning keep their offset as name.
class GameApp {
public:
    // 0x004DF660 — __thiscall, (event, param), returns GameEventStatus or the
    // event number itself for events 1, 0x25 and 0x26.
    int HandleEvent(int event, void* param);
    // 0x004DF5C0 — event 0x18; always returns 0.
    int SetDefaults();
    // 0x004DF610 — event 0x15; returns 1 on success (GameAppInit.cpp).
    int Initialise(void* param);
    // 0x004DF450 — called by MH2_InitGame; returns 1 on success (GameAppInit.cpp).
    int Load(int unused);
    // 0x004DF4A0 — event 0x16 (not decompiled yet).
    int Shutdown();

    const char*    appName;      // +0x00  "MANHUNT"
    int32_t        width;        // +0x04  640
    int32_t        height;       // +0x08  480 (from g_DefaultScreenHeight)
    GameSubObject* sub;          // +0x0C  set by MH2_InitGame, not by SetDefaults
    int32_t        width2;       // +0x10  640
    int32_t        height2;      // +0x14  480
    int32_t        frameLimit;   // +0x18  30 (?)
    int32_t        flag1C;       // +0x1C  set by event 0x1E
    int32_t        unk20;        // +0x20  never written here
    int32_t        unk24;        // +0x24
    int32_t        unk28;        // +0x28
    int32_t        unk2C;        // +0x2C
    int32_t        unk30;        // +0x30  1
    int32_t        unk34;        // +0x34
    int32_t        unk38;        // +0x38
    int32_t        unk3C;        // +0x3C  2
    int32_t        unk40;        // +0x40
    int32_t        unk44;        // +0x44
    void*          memory48;     // +0x48  from Unknown_00580ED0(1) in Initialise
    bool           initialised;  // +0x4C
};

#if defined(_M_IX86)
static_assert(offsetof(GameApp, sub) == 0x0C, "GameApp layout");
static_assert(offsetof(GameApp, frameLimit) == 0x18, "GameApp layout");
static_assert(offsetof(GameApp, memory48) == 0x48, "GameApp layout");
static_assert(offsetof(GameApp, initialised) == 0x4C, "GameApp layout");
#endif

// 0x00697658 — 480 in the original's data; read by SetDefaults.
extern int32_t g_DefaultScreenHeight;

// 0x006EBF80 — 12-byte version string buffer ("0.9999" at boot).
extern char g_VersionString[12];

// 0x00424990 (called through thunk 0x004B0230) — copies at most 11 chars and
// always leaves the buffer terminated.
void SetVersionString(const char* version);

// 0x004202E6 (via thunk 0x004DF730) — returns 1, nothing else. Its body
// jumps back into the thunk's own ret, i.e. code SecuROM relocated; most
// likely a check that was emptied out.
int AlwaysTrue();
