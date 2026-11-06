// Paths.h — the game's file path table and the start-up step that applies it.
#pragma once

struct GameString;  // the engine's string type (ctor 0x005FDA60, dtor 0x005FDB70)

// One record of the path table at 0x006A2A48 (12 bytes on x86). The table
// ends with a record whose slot is null. Several records may name the same
// slot; later ones override earlier ones (console base paths first, PC
// overrides after).
struct PathRecord {
    GameString* slot;
    const char* pathA;  // used on PC: often a packed resourceNN.glg
    const char* pathB;  // the loose text name (LEVELS.txt, AITYPEDATA.ini...)
};

// 0x006A2A48 — 155 records + terminator in the original. Read from the
// user's game data at run time by the port, never copied into this repo.
extern const PathRecord* g_PathTable;

// 0x0053CF50 (called through 0x0053ABD0 with usePathA = true, as the first
// step of MH2_InitGame) — assigns every record's path to its slot, in order.
void Paths_Init(bool usePathA);
