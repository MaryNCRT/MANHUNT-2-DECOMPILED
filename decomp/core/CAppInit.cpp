// CAppInit.cpp — CApp_Init.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   CApp_Init  0x007D27F0  readable; read in execution order with
//              `disasm.py ... follow`: SecuROM split it into pieces joined by
//              jmps, so a linear listing (and Ghidra's draft) cuts it short.
//
// Ghidra's draft is wrong in two places, fixed here from the disassembly:
// 0x005BF6C0 returns an object that 0x005BF660 is called on (the draft showed
// an unrelated thunk call), and the same for 0x005B8D00 / 0x005B9BD0.

#include "CApp.h"
#include "original.h"

bool CApp_Init()
{
    SetLoadingMessage("Initialising CApp");
    Paths_SetupFromGlobals_007D2570();      // via thunk 0x004B0270 (?)

    // Parses global/initscripts/LEVELS.txt (the path slot at 0x007597E4) with
    // the "LEVEL" / "ENABLE_LEVEL_ORDER" handler.
    // ORIGINAL LIMIT L3: the reader takes at most 80,000 bytes of the file.
    TextScript_Parse(g_PathLevelsTxt, LevelList_OnToken);   // 0x00541F90, handler 0x00546D90

    Unknown_00608680();                     // zeroes 0x14C bytes at 0x0079D118, allocates 0xC000
    if (!Unknown_005898C0(g_Unknown_0075B6F0))  // sets render/light defaults between checks (?)
        return false;

    Unknown_005DECF0();
    Unknown_005F3F30();
    Unknown_007D2640();                     // via thunk 0x004B02F0
    Pool32_Init_005F1E90();                 // fixed pool of 32 records of 0x54 bytes
    Unknown_005BF660(Unknown_005BF6C0());   // object from 0x005BF6C0, then a method on it

    // A 0xF620-byte singleton; if the allocation fails it stays null and the
    // start-up carries on.
    if (g_Vec4Table == nullptr) {
        void* mem = Mem_Alloc(sizeof(Vec4Table_F620));
        g_Vec4Table = mem ? static_cast<Vec4Table_F620*>(mem)->Construct() : nullptr;
    }

    Unknown_005B9BD0(Unknown_005B8D00());   // 0x12330-byte singleton; allocates a 1 MB buffer
    ResetGlobals_0076BD80();
    return true;
}
