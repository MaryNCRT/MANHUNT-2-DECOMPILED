// GameAppInit.cpp — GameApp::Initialise and GameApp::Load.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   GameApp::Initialise  0x004DF610  (76 B)  readable; call order and ECX checked in asm
//   GameApp::Load        0x004DF450  (67 B)  readable; call order and ECX checked in asm
//   GameApp_VCallSub1C   0x004DF5B0  (10 B)  readable
//
// Both are __thiscall with one stack argument that they ignore (ret 4).

#include "GameApp.h"
#include "original.h"

// 0x004DF5B0 — tail call to sub->vtable[0x1C / 4]().
static int GameApp_VCallSub1C(GameApp* app)
{
    return SubObject_VCall1C(app->sub);
}

int GameApp::Initialise(void* /*param*/)
{
    // The object sends itself event 4 first; that event is always processed.
    if (!HandleEvent(kEvent_04, nullptr))
        return 0;
    if (!GameApp_VCallSub1C(this))
        return 0;
    // UNRESOLVED: 0x00580ED0 also hands out the 15 MB FMOD block and the
    // 0x188- and 0xF620-byte objects, so it looks like an allocator, but here
    // it is called with 1; whether that is a size or a heap id is open.
    memory48 = Unknown_00580ED0(1);
    if (memory48 == nullptr)
        return 0;
    SetVersionString("0.9999");
    initialised = true;
    return 1;
}

int GameApp::Load(int /*unused*/)
{
    Language_Load(g_Language_0075FC9C);                 // 0x0056B340 (?)
    OptionsTable_RegisterAll(OptionsTable_Get());       // 0x00561C50 on the 0x00562DF0 singleton
    SaveGames_Init(g_SaveGames_007605F0);               // 0x0056C3D0 (?)

    // Ghidra marked this branch as never returning; the disassembly shows it
    // frees the block and returns 0 like any other failure.
    if (!CApp_Init()) {                                 // 0x007D27F0 via 0x004B12D0, "Initialising CApp"
        Unknown_00580EF0(memory48);
        return 0;
    }
    Unknown_005E02A0();
    return 1;
}
