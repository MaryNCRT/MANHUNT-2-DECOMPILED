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

// 0x006154F0 — game initialization (subsystems, see PROGRESS.md)
bool MH2_InitGame();
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

// 0x0079E700 — main game object; its vtable is called during init/shutdown
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
