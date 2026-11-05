// original.h — functions and globals of the original executable that are
// referenced by decompiled code but not decompiled yet.
//
// Every entry names its address in Manhunt2.exe (build 0x4992F67C, see
// PROGRESS.md §2). When a function gets decompiled, its declaration moves to
// the header of its own module and is removed from here.
//
// Names come from what the code does, or from the public DXUT API where the
// function is a DXUT routine. Names marked "(?)" are not confirmed yet.

#pragma once

#include <windows.h>
#include <cstdint>

// ---------------------------------------------------------------------------
// DXUT (DirectX SDK sample framework). The port replaces DXUT with its own
// platform layer instead of decompiling it; the addresses are kept to map the
// callbacks the game registers.
// ---------------------------------------------------------------------------

// 0x0040A590
HRESULT DXUTInit(bool parseCommandLine, bool showMsgBoxOnError,
                 const WCHAR* extraCommandLineParams, bool threadSafe);
// 0x004083D0
void DXUTSetHotkeyHandling(bool altEnterToggleFullscreen, bool escapeToQuit,
                           bool pauseToToggleTimePause);
// 0x00408480
void DXUTSetCursorSettings(bool showCursorWhenFullScreen, bool clipCursorWhenFullScreen);
// 0x00403F30 — writes the same DXUT state field as DXUTInit's showMsgBoxOnError (?)
void DXUTSetShowMsgBoxOnError(bool show);
// 0x007A70D0 (relocated into .secu)
HRESULT DXUTCreateWindow(const WCHAR* title, HINSTANCE instance, HICON icon, HMENU menu,
                         int x, int y);
// 0x0040DC80 — DXUTCreateDevice extended by Rockstar with refresh rate and
// one more parameter (?)
HRESULT MH2_CreateDevice(bool windowed, int width, int height, int refreshRate, int unknown);
// 0x0040E3A0
HRESULT DXUTMainLoop(HACCEL accel);
// 0x0044C1A0 (via thunk 0x004043F0)
int DXUTGetExitCode();
// 0x00408620 — returns the window handle; used before ShowWindow(SW_HIDE) (?)
HWND DXUTGetHWND_Maybe();

// Callback registration (one setter per DXUT state slot). The slot each one
// fills is known from the state offset it writes; the DXUT name is (?).
typedef void* DxutCallback;
void DXUTSetCallback_0358(DxutCallback cb, void* userContext);  // 0x004044F0
void DXUTSetCallback_A(DxutCallback cb, void* userContext);     // 0x00404570
void DXUTSetCallback_B(DxutCallback cb, void* userContext);     // 0x004045F0
void DXUTSetCallback_C(DxutCallback cb);                        // 0x00404670
void DXUTSetCallback_D(DxutCallback cb, void* userContext);     // 0x004046C0
void DXUTSetCallback_E(DxutCallback cb, void* userContext);     // 0x00404740
void DXUTSetCallback_F(DxutCallback cb, void* userContext);     // 0x004047C0
void DXUTSetCallback_G(DxutCallback cb, void* userContext);     // 0x00404840
void DXUTSetCallback_H(DxutCallback cb, void* userContext);     // 0x004048C0
void DXUTSetCallback_I(DxutCallback cb, void* userContext);     // 0x00404940
void DXUTSetCallback_J(DxutCallback cb, void* userContext);     // 0x004049C0

// Game callbacks handed to DXUT (all next to WinMain in the binary).
void MH2_Callback_006151D0();
void MH2_Callback_00615250();
void MH2_Callback_00615260();
void MH2_Callback_00615270();
void MH2_Callback_006152B0();
void MH2_Callback_006152F0();
void MH2_Callback_00615330();
void MH2_Callback_00615360();
void MH2_Callback_006153B0();
void MH2_Callback_006153F0();
void MH2_Callback_00615410();

// ---------------------------------------------------------------------------
// Game
// ---------------------------------------------------------------------------

// 0x006154F0 — game initialization, decompiled in GameInit.cpp
bool MH2_InitGame();

// Callees of MH2_InitGame, in call order.
void Unknown_0053ABD0();
void Unknown_0060DA80(bool layout);
void Video_ProbeMultisampleModes();       // 0x00606E40: keeps which of 2/4/8/16 pass 0x0040A510 (?)
int  Unknown_004202E6(int);               // returns 1
int  Unknown_0060D420(bool larger, int);  // 480x272 or 512x320 surface setup
void Clock_Start();                       // 0x004B1820 (?)
void Clock_ResetFrameCounters();          // 0x007D2A70 (?)
void GameSubObject_VCall(int vtableOffset);  // stands for g_Game->sub->vtable[offset/4]()
void Unknown_0056F990();
struct Settings;
extern Settings* g_Settings;              // 0x007604C8, static settings object
void Settings_ClampDetailLevel(Settings* self);  // 0x0056C070 (?)
// GameApp::Initialise (0x004DF610) and GameApp::Load (0x004DF450) are decompiled
// in GameApp.h / GameAppInit.cpp.
bool Game_Init_004F8C40(uint8_t* game, int);   // __thiscall; body is "return 1"

// Callees of GameApp::Initialise / GameApp::Load.
struct GameSubObject;
int   SubObject_VCall1C(GameSubObject* sub);     // sub->vtable[0x1C / 4]()
void* Unknown_00580ED0(unsigned sizeOrId);       // 0x00580E30 then 0x00580C30 on its result
void  Unknown_00580EF0(void* p);                 // 0x00580E30 then 0x00580D80; null-safe
struct LanguageTable;
extern LanguageTable* g_Language_0075FC9C;       // static object at 0x0075FC9C
void  Language_Load(LanguageTable* self);        // 0x0056B340 (?): picks data by g_Language
struct OptionsTable;
OptionsTable* OptionsTable_Get();                // 0x00562DF0: function-local static at 0x0075F110
void  OptionsTable_RegisterAll(OptionsTable*);   // 0x00561C50: "soundType", "subtitles", "volumeSFX"...
struct SaveGames;
extern SaveGames* g_SaveGames_007605F0;          // static object at 0x007605F0
void  SaveGames_Init(SaveGames* self);           // 0x0056C3D0 (?): copies 0xAD08 bytes, "EMPTY" slot
bool  CApp_Init();                               // 0x007D27F0 via 0x004B12D0: logs "Initialising CApp"
void  Unknown_005E02A0();
extern uint8_t* g_Object_00696ED8;
void Unknown_004C5390(uint8_t* self);
extern uint8_t g_GameObject[];            // 0x0079E728
extern uint8_t g_GameSubObject[];         // 0x0079E70C
// 0x0053CC10
bool FileExists(const char* path);
// 0x0056C2D0 — load saved settings; false if missing or invalid (?)
bool Settings_Load();
// 0x0056C190 — reset settings to defaults (?)
void Settings_SetDefaults();
// 0x00618EB0 — 145 bytes, 120 callers, no callees; used on the saved
// resolution before device creation. Not identified yet, arity unknown.
int Unknown_00618EB0(int a, ...);

// 0x0056F560 — called right before DXUTMainLoop (?)
void MH2_BeforeMainLoop_0056F560();

// Shutdown sequence after the main loop, in call order (all (?)).
void MH2_Shutdown_0056F9A0();
void MH2_Shutdown_0056C0C0();
void MH2_Shutdown_00594420();
void MH2_Shutdown_005943D0();
void MH2_Shutdown_004DF4A0();
void MH2_Shutdown_004CE710();

// 0x00580E30 / 0x00580C30 — together they hand out the FMOD memory block.
// 0x00580E30 takes a size and returns an object; 0x00580C30 is __thiscall on
// that object and returns the pointer. Memory manager, not decompiled yet.
void* Unknown_00580E30(unsigned size);
void* Unknown_00580C30(void* self);

// 0x007605DC — UI language index: 0 EN, 1 DE, 2 FR, 3 IT, 4 ES
extern int g_Language;
// 0x00704808 (?) — ten 0x38-byte audio records reset by Audio_Init
extern uint8_t g_AudioRecords[10 * 0x38];

// 0x0079E700 — pointer to the main game object. MH2_InitGame sets it to the
// static object at 0x0079E728, and sets that object's field +0xC
// (0x0079E734) to the sub-object at 0x0079E70C. Its vtable is called during
// init and shutdown.
extern uint8_t* g_Game;
// 0x0079E704 — set when global/pictures/legal_fr.tex is missing (non-French build)
extern int g_NoFrenchLegalScreen;
// 0x007604D0 / 0x007604F4 — saved video mode values passed to device creation
extern int g_SavedVideoA;
extern int g_SavedVideoB;
// 0x007604D4 — saved "fullscreen" flag (device is windowed when 0)
extern char g_SavedFullscreen;
// 0x007604E4 / 0x007604EC — cleared by -stereo (?)
extern int g_Settings_7604E4;
extern int g_Settings_7604EC;
// 0x007604F8 — stereo 3D mode requested on the command line
extern char g_StereoMode;
