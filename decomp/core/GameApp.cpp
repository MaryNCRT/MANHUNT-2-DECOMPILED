// GameApp.cpp — event handler and defaults of the main game object.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   GameApp::HandleEvent  0x004DF660  (127 B)  verified except events 0x15 and 0x16
//   GameApp::SetDefaults  0x004DF5C0  ( 76 B)  verified
//   SetVersionString      0x00424990  ( 30 B)  verified
//   AlwaysTrue            0x004202E6  ( 11 B)  verified
// The handler's jump table (0x004DF6E0, index bytes at 0x004DF700) was decoded
// by hand; Ghidra's switch matches it.

#include "GameApp.h"

#include <cstring>

int32_t g_DefaultScreenHeight = 480;
char g_VersionString[12];

int AlwaysTrue()
{
    return 1;
}

void SetVersionString(const char* version)
{
    g_VersionString[11] = '\0';
    std::strncpy(g_VersionString, version, 11);
}

int GameApp::SetDefaults()
{
    appName    = "MANHUNT";
    width      = 640;
    height     = g_DefaultScreenHeight;
    width2     = 640;
    height2    = g_DefaultScreenHeight;
    frameLimit = 30;
    flag1C     = 0;
    unk24 = unk28 = unk2C = 0;
    unk30      = 1;
    unk34 = unk38 = 0;
    unk3C      = 2;
    unk40 = unk44 = 0;
    return 0;  // so event 0x18 always reports kEventError
}

int GameApp::HandleEvent(int event, void* param)
{
    if (event == kEvent_SetsFlag1C)
        flag1C = 1;

    switch (event) {
    case kEvent_ReturnsItself1:
    case kEvent_ReturnsItself25:
    case kEvent_ReturnsItself26:
        return event;
    case kEvent_04:
    case kEvent_19:
        return kEventProcessed;
    case kEvent_0A:
    case kEvent_17:
        return AlwaysTrue() != 0;
    case kEvent_Initialise:
        return Initialise(param) != 0;
    case kEvent_Shutdown:
        Shutdown();
        return kEventProcessed;
    case kEvent_SetDefaults:
        return SetDefaults() != 0;
    default:
        return kEventNotProcessed;
    }
}
