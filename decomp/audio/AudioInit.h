// AudioInit.h — FMOD start-up and the fatal-error box. See AudioInit.cpp.
#pragma once

#include <windows.h>

namespace FMOD { class EventSystem; class System; }

extern FMOD::EventSystem* g_EventSystem;  // 0x00700FB4
extern FMOD::System*      g_FmodSystem;   // 0x00700FB0

// 0x004CE4E0 — starts FMOD; on failure shows "Sound Error" and exits.
void Audio_Init();

// 0x004CE4B0 — hides the window, shows a message box, exits the process.
[[noreturn]] void FatalError(const WCHAR* text, const WCHAR* caption);
