// Paths.cpp — Paths_Init.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   Paths_Init        0x0053CF50  (157 B)  readable; every step from the disassembly
//   Paths_InitForPC   0x0053ABD0  (  9 B)  readable
//
// Not verified yet: it writes the engine's string objects, whose methods
// (0x005FDD60, 0x005FDCF0, 0x005FDC90) are not decompiled, and calls
// 0x0053CE10. The oracle needs those first.

#include "Paths.h"
#include "original.h"

#include <cstring>

// 0x005FDD60 / 0x005FDCF0 / 0x005FDC90 — methods of the string type.
// Argument order of the second one is (length, text): the original pushes the
// text first, then 34, the length of the replacement path.
void GameString_Assign(GameString* self, const char* text);
void GameString_AssignN(GameString* self, int length, const char* text);
void GameString_Op5FDC90(GameString* self, int value);

extern GameString* g_PathSlot_00759794;
void Unknown_0053CE10();

void Paths_Init(bool usePathA)
{
    // The original first walks the table to its end and throws the count
    // away; that loop has no effect and is left out.
    for (const PathRecord* r = g_PathTable; r->slot != nullptr; ++r) {
        if (!usePathA) {
            GameString_Assign(r->slot, r->pathB);
            continue;
        }
        // The one special case: compares 16 bytes, i.e. the full
        // "controller_page" including its terminator.
        if (std::memcmp(r->pathA, "controller_page", 16) == 0) {
            static const char kControllerPicture[] = "global/pictures/MH2_controller.tex";
            GameString_AssignN(r->slot, sizeof(kControllerPicture) - 1, kControllerPicture);
        } else {
            GameString_Assign(r->slot, r->pathA);
        }
    }
    Unknown_0053CE10();
    GameString_Op5FDC90(g_PathSlot_00759794, 0x40);
}

// 0x0053ABD0 — what MH2_InitGame calls first.
void Paths_InitForPC()
{
    Paths_Init(true);
}
