// CApp.cpp — CApp_Init and the parts of it that are decompiled.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   SetLoadingMessage         0x004B0570  ( 10 B)  verified (tests/oracle/test_capp.cpp)
//   Vec4Table_F620::Construct 0x004B08C0  ( 82 B)  verified
//   ResetGlobals_0076BD80     0x0056EFB0  (193 B)  verified
// CApp_Init (0x007D27F0) is in CAppInit.cpp.

#include "CApp.h"

const char* g_LoadingMessage = nullptr;
Globals_0076BD80 g_Globals_0076BD80;

void SetLoadingMessage(const char* message)
{
    g_LoadingMessage = message;
}

Vec4Table_F620* Vec4Table_F620::Construct()
{
    for (auto& r : a)
        r[0] = r[1] = r[2] = r[3] = 0.0f;
    for (auto& r : b)
        r[0] = r[1] = r[2] = r[3] = 0.0f;
    unkF618 = 0;
    unkF61C = 0;
    return this;
}

void ResetGlobals_0076BD80()
{
    for (int32_t& w : g_Globals_0076BD80.words)
        w = 0;
    g_Globals_0076BD80.at0076BE40 = 0;
    g_Globals_0076BD80.at0076BE50 = 0;
}
