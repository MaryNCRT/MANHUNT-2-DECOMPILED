// AudioInit.cpp — FMOD Ex / FMOD Event System start-up.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   Audio_Init                0x004CE4E0  (545 B)  readable, call arguments checked in asm
//   Audio_TrackPeakMemory     0x004CDEB0  ( 39 B)  readable (body continues at 0x004CDED8)
//   FatalError                0x004CE4B0  ( 45 B)  readable
//   SoundErrorText/Caption    0x004CE4A0 / 0x0040DC68  readable
//
// The sequence follows FMOD Ex 4.x's own sample start-up code step by step, so
// the FMOD calls use the public API names. Arguments were read from the
// disassembly, not from Ghidra's draft, which mixed up FMOD's stack arguments.

#include "AudioInit.h"
#include "../core/original.h"

#include <fmod.hpp>
#include <fmod_event.hpp>
#include <fmod_errors.h>
#include <cstdlib>

// 0x00700FB4 / 0x00700FB0
FMOD::EventSystem* g_EventSystem = nullptr;
FMOD::System*      g_FmodSystem  = nullptr;
// 0x00700FB8 — memory block handed to FMOD
void*              g_FmodPool    = nullptr;
// 0x00700FC4 — highest FMOD memory use seen
static int         s_FmodPeakBytes = 0;

// ORIGINAL LIMIT: FMOD gets one fixed 15 MB block for everything it allocates
// (event data, streams, DSP buffers). Audio mods that add or enlarge banks run
// out of it. Recorded in docs/LIMITS.md.
static const unsigned kFmodPoolBytes = 0xF00000;

// 0x004CDEB0 — called after each FMOD call that failed. Despite the call
// sites, it only records peak memory; it does not report the error.
static void Audio_TrackPeakMemory()
{
    int current = 0, max = 0;
    if (FMOD_Memory_GetStats(&current, &max) == FMOD_OK && current > s_FmodPeakBytes)
        s_FmodPeakBytes = current;  // (?) exact comparison at 0x004CDED8 not decompiled yet
}

// 0x0040DC68 / 0x004CE4A0 — localized "Sound Error" text and caption, from
// the table at 0x00696F80 {text, caption}, indexed by g_Language (0x007605DC).
extern const WCHAR* const kSoundErrorTable[][2];  // 0x00696F80
static const WCHAR* SoundErrorText()    { return kSoundErrorTable[g_Language][0]; }
static const WCHAR* SoundErrorCaption() { return kSoundErrorTable[g_Language][1]; }

// 0x004CE4B0 — hide the window, show the message, exit(0). Never returns.
[[noreturn]] void FatalError(const WCHAR* text, const WCHAR* caption)
{
    if (HWND wnd = DXUTGetHWND_Maybe())
        ShowWindow(wnd, SW_HIDE);
    MessageBoxW(nullptr, text, caption, MB_ICONERROR);
    std::exit(0);
}

// PORT: in the original every failure below is fatal: no audio device, a
// headset unplugged at boot, or an unusual driver closes the game with
// "Sound Error". The port will fall back to FMOD_OUTPUTTYPE_NOSOUND and keep
// running. Not implemented yet: this file mirrors the original for now.
static void SoundInitFailed()
{
    FatalError(SoundErrorText(), SoundErrorCaption());
}

void Audio_Init()
{
    // UNRESOLVED: two calls produce the block: 0x00580E30(size) returns an
    // object, then 0x00580C30 (__thiscall on it) returns the pointer.
    g_FmodPool = Unknown_00580C30(Unknown_00580E30(kFmodPoolBytes));
    FMOD_Memory_Initialize(g_FmodPool, kFmodPoolBytes, nullptr, nullptr, nullptr);

    if (FMOD::EventSystem_Create(&g_EventSystem) != FMOD_OK)
        Audio_TrackPeakMemory();
    if (g_EventSystem->getSystemObject(&g_FmodSystem) != FMOD_OK)
        Audio_TrackPeakMemory();

    unsigned version = 0;
    if (g_FmodSystem->getVersion(&version) != FMOD_OK)
        Audio_TrackPeakMemory();

    if (g_FmodSystem->setSoftwareChannels(64) != FMOD_OK)
        Audio_TrackPeakMemory();

    FMOD_CAPS caps = 0;
    FMOD_SPEAKERMODE speakerMode = FMOD_SPEAKERMODE_STEREO;
    if (g_FmodSystem->getDriverCaps(0, &caps, nullptr, nullptr, &speakerMode) != FMOD_OK)
        SoundInitFailed();

    // Emulated (software-only) drivers get a larger buffer to avoid crackling.
    if (caps & FMOD_CAPS_HARDWARE_EMULATED) {
        if (g_FmodSystem->setDSPBufferSize(1024, 10) != FMOD_OK) {
            Audio_TrackPeakMemory();
            Audio_TrackPeakMemory();
        }
    }

    // Use the speaker setup chosen in the Windows control panel.
    if (g_FmodSystem->setSpeakerMode(speakerMode) != FMOD_OK)
        SoundInitFailed();

    if (g_FmodSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCM16, 0, 14,
                                        FMOD_DSP_RESAMPLER_LINEAR) != FMOD_OK)
        SoundInitFailed();

    // init(maxchannels 64, flags 2, extradriverdata, eventflags). Flag value 2 is
    // FMOD_INIT_3D_RIGHTHANDED in FMOD Ex 4.x headers (?), to confirm against
    // the exact FMOD version the game shipped with.
    FMOD_RESULT result = g_EventSystem->init(64, (FMOD_INITFLAGS)2, nullptr, FMOD_EVENT_INIT_NORMAL);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER) {
        // The control-panel speaker mode is not supported by this card:
        // fall back to stereo and try again (same as FMOD's sample code).
        if (g_FmodSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO) != FMOD_OK)
            Audio_TrackPeakMemory();
        result = g_EventSystem->init(64, (FMOD_INITFLAGS)2, nullptr, FMOD_EVENT_INIT_NORMAL);
        if (result != FMOD_OK) {
            Audio_TrackPeakMemory();
            SoundInitFailed();
        }
    } else if (result != FMOD_OK) {
        SoundInitFailed();
    }

    FMOD_Debug_SetLevel(0);

    // ORIGINAL LIMIT (?): ten fixed records of 0x38 bytes. Two fields of each
    // (first at 0x0070480C / 0x00704810) are reset to 0 and 0x13. What the
    // records hold is not known yet.
    for (int i = 0; i < 10; ++i) {
        uint8_t* rec = g_AudioRecords + i * 0x38;
        *reinterpret_cast<int32_t*>(rec + 0) = 0;
        *reinterpret_cast<int32_t*>(rec + 4) = 0x13;
    }

    Audio_TrackPeakMemory();
}
