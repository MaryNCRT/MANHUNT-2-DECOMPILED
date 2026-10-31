// GameInit.cpp — game start-up called by WinMain once the D3D device exists.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   MH2_InitGame   0x006154F0  (198 B)  readable, call order and ECX of every
//                                       call checked in the disassembly
//
// Most callees are not decompiled yet. Each is named only as far as the
// evidence goes; the rest keep their address. Call order is the original's.

#include "original.h"
#include "../audio/AudioInit.h"

bool MH2_InitGame()
{
    // PORT: the original starts with
    //     SetProcessAffinityMask(GetCurrentProcess(), 1);
    // which pins the whole game to CPU core 0 for its lifetime. On modern
    // systems it costs performance and makes the game share one core with
    // FMOD's and the driver's threads. The port does not do it. This is the
    // fix the mh2-affinity plugin applies by patching these bytes.

    Unknown_0053ABD0();               // calls 0x0053CF50(1)
    Unknown_0060DA80(true);           // lays out a list of blocks (std::vector, checked iterators)
    Video_ProbeMultisampleModes();    // 0x00606E40 (?)
    Unknown_004202E6(0);              // via thunk 0x004DF730; body is "return 1"
    // 0x0060D420: first argument picks a 480x272 (false) or 512x320 (true)
    // surface size, i.e. PSP resolutions; the second is passed through.
    Unknown_0060D420(false, 1);

    // The main game object is a static at 0x0079E728; g_Game points to it.
    g_Game = g_GameObject;
    *reinterpret_cast<uint8_t**>(g_GameObject + 0xC) = g_GameSubObject;  // 0x0079E70C

    Clock_Start();                    // 0x004B1820 (?) reads a time value via the sub-object
    Clock_ResetFrameCounters();       // 0x007D2A70 via thunk 0x004B19C0 (?)
    GameSubObject_VCall(0x10);        // g_Game->sub->vtable[0x10 / 4]()

    Unknown_0056F990();               // three calls: 0x56F530, 0x56F420, 0x56F3B0
    Settings_ClampDetailLevel(g_Settings);  // 0x0056C070, __thiscall on 0x007604C8 (?)

    if (!Game_Init_004DF610(g_Game, 0))     // __thiscall; uses the string "0.9999"
        return false;

    Audio_Init();

    if (!Game_Init_004DF450(g_Game, 0))     // __thiscall; loads 4 things, then a check
        return false;

    Game_Init_004F8C40(g_Game, 0);          // __thiscall; body is "return 1"
    Unknown_004C5390(g_Object_00696ED8);    // __thiscall, via thunk to 0x007D5600
    GameSubObject_VCall(0x0C);              // g_Game->sub->vtable[0x0C / 4]()
    return true;
}
