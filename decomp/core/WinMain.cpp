// WinMain.cpp — process entry, single-instance check, DXUT setup, shutdown.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   WinMain              0x006155C0  (595 B)  readable, not verified
//   CountProcessesNamed  0x00615420  (197 B)  readable, return value verified in asm
//
// Differences from the original are marked "PORT:". Everything else mirrors
// the original control flow.

#include "original.h"

#include <tlhelp32.h>
#include <cwchar>

// Number of running processes whose image name contains `name`
// (case-sensitive substring, like the original's wcsstr call).
// Returns 0 if `name` is null or the snapshot fails.
static unsigned CountProcessesNamed(const WCHAR* name)
{
    if (name == nullptr)
        return 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    unsigned count = 0;
    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);
    for (BOOL ok = Process32FirstW(snapshot, &entry); ok; ok = Process32NextW(snapshot, &entry)) {
        if (wcsstr(entry.szExeFile, name) != nullptr)
            ++count;
    }
    CloseHandle(snapshot);
    return count;
}

// "Manhunt 2 is already running." in the five shipped languages.
// Index: 0 English, 1 German, 2 French, 3 Italian, 4 Spanish.
// The original reads these from a table at 0x006DC290 (message, caption).
static int UiLanguageIndex()
{
    switch (PRIMARYLANGID(GetUserDefaultUILanguage())) {
    case LANG_GERMAN:  return 1;
    case LANG_FRENCH:  return 2;
    case LANG_ITALIAN: return 3;
    case LANG_SPANISH: return 4;
    default:           return 0;
    }
}

extern const WCHAR* const kAlreadyRunningText[5][2];  // 0x006DC290 {message, caption}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR cmdLine, int)
{
    // Our own process is always counted, so more than one means another copy runs.
    if (CountProcessesNamed(L"Manhunt2") > 1) {
        const int lang = UiLanguageIndex();
        MessageBoxW(nullptr, kAlreadyRunningText[lang][0], kAlreadyRunningText[lang][1],
                    MB_ICONERROR);
        return 0;
    }

    // Command-line switches.
    const bool safeMode   = cmdLine != nullptr && wcsstr(cmdLine, L"-safe") != nullptr;
    const bool stereoMode = cmdLine != nullptr && wcsstr(cmdLine, L"-stereo") != nullptr;

    // Hand the game's callbacks to DXUT.
    DXUTSetCallback_E(MH2_Callback_006151D0, nullptr);
    DXUTSetCallback_F(MH2_Callback_00615260, nullptr);
    DXUTSetCallback_G(MH2_Callback_00615270, nullptr);
    DXUTSetCallback_J(MH2_Callback_00615360, nullptr);
    DXUTSetCallback_H(MH2_Callback_006152B0, nullptr);
    DXUTSetCallback_I(MH2_Callback_006152F0, nullptr);
    DXUTSetCallback_0358(MH2_Callback_00615250, nullptr);
    DXUTSetCallback_D(MH2_Callback_00615410, nullptr);
    DXUTSetCallback_A(MH2_Callback_00615330, nullptr);
    DXUTSetCallback_B(MH2_Callback_006153B0, nullptr);
    DXUTSetCallback_C(MH2_Callback_006153F0);

    // The French legal screen only ships in the French build.
    if (!FileExists("global/pictures/legal_fr.tex"))
        g_NoFrenchLegalScreen = 1;

    // -safe ignores the saved settings; a failed load does the same.
    if (safeMode || !Settings_Load())
        Settings_SetDefaults();

    if (stereoMode) {
        g_Settings_7604EC = 0;
        g_Settings_7604E4 = 0;
    }
    g_StereoMode = stereoMode;

    DXUTInit(true, true, nullptr, false);
    DXUTSetHotkeyHandling(false, false, false);
    DXUTSetCursorSettings(true, true);
    DXUTCreateWindow(L"Manhunt 2", nullptr, nullptr, nullptr, CW_USEDEFAULT, CW_USEDEFAULT);
    DXUTSetShowMsgBoxOnError(false);

    // UNRESOLVED: Ghidra shows Unknown_00618EB0 called first with two arguments,
    // then with one; its real arity and meaning are still open, so the device
    // size arguments below are not trustworthy yet.
    const int a = Unknown_00618EB0(g_SavedVideoA, g_SavedVideoB);
    const int b = Unknown_00618EB0(a);
    if (FAILED(MH2_CreateDevice(g_SavedFullscreen == 0, b, a, g_SavedVideoA, g_SavedVideoB)))
        return 0;

    // PORT: MH2_InitGame starts with SetProcessAffinityMask(process, 1), which
    // pins the whole game to one CPU core. The port drops that call.
    MH2_InitGame();

    MH2_BeforeMainLoop_0056F560();
    DXUTMainLoop(nullptr);
    MH2_Shutdown_0056F9A0();
    MH2_Shutdown_0056C0C0();
    *reinterpret_cast<int*>(g_Game + 0x20) = 1;
    MH2_Shutdown_00594420();
    MH2_Shutdown_005943D0();
    MH2_Shutdown_004DF4A0();
    MH2_Shutdown_004CE710();
    return DXUTGetExitCode();
}
