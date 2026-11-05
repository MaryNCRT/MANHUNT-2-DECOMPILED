// test_gameapp.cpp — differential tests for decomp/core/GameApp.cpp.
#include "oracle.h"
#include "pe_image.h"

#include "../../decomp/core/GameApp.h"

#include <cstring>
#include <string>

using namespace oracle;

namespace {

// __thiscall from outside a class: __fastcall puts the first argument in ECX
// and the second in EDX, and the callee cleans the stack like __thiscall.
using HandleEventFn = int(__fastcall*)(void* self, void* edx, int event, void* param);
using SetDefaultsFn = int(__fastcall*)(void* self, void* edx);

constexpr size_t kAppSize = 0x50;
static_assert(sizeof(GameApp) == kAppSize, "GameApp must match the original's 0x50 bytes");

int32_t& OriginalDefaultHeight() { return *At<int32_t*>(0x00697658); }
char* OriginalVersionBuffer() { return At<char*>(0x006EBF80); }

// Same random bytes in both objects, with the name pointer cleared.
void Fill(uint8_t* a, uint8_t* b, Rng& rng)
{
    for (size_t i = 0; i < kAppSize; ++i)
        a[i] = b[i] = static_cast<uint8_t>(rng.Next());
    std::memset(a, 0, sizeof(void*));
    std::memset(b, 0, sizeof(void*));
}

// Compares the two objects. The name pointer is compared by its text, since
// each binary has its own "MANHUNT" literal.
void CompareObjects(const uint8_t* orig, const uint8_t* ours, const std::string& input)
{
    const char* on = *reinterpret_cast<const char* const*>(orig);
    const char* dn = *reinterpret_cast<const char* const*>(ours);
    const bool sameName = (on == nullptr && dn == nullptr) ||
                          (on != nullptr && dn != nullptr && std::strcmp(on, dn) == 0);
    Compare(sameName ? 1 : 0, 1, input + " name");
    for (size_t i = sizeof(void*); i < kAppSize; ++i)
        Compare(orig[i], ours[i], input + " byte +" + std::to_string(i));
}

}  // namespace

// 0x004DF5C0, __thiscall. Reads the original's 0x00697658 for the height, so
// both sides get the same value set before each case.
ORACLE_TEST(GameApp_SetDefaults, "0x004DF5C0")
{
    auto original = At<SetDefaultsFn>(0x004DF5C0);
    Rng rng(0x4DF5C0);
    const int32_t savedHeight = OriginalDefaultHeight();
    for (int i = 0; i < 2000; ++i) {
        const int32_t h = (i == 0) ? 480 : static_cast<int32_t>(rng.Next());
        OriginalDefaultHeight() = h;
        g_DefaultScreenHeight = h;

        alignas(8) uint8_t orig[kAppSize];
        alignas(8) uint8_t ours[kAppSize];
        Fill(orig, ours, rng);
        const int ro = original(orig, nullptr);
        const int rd = reinterpret_cast<GameApp*>(ours)->SetDefaults();
        const std::string label = "height " + std::to_string(h);
        Compare(ro, rd, label + " return");
        CompareObjects(orig, ours, label);
    }
    OriginalDefaultHeight() = savedHeight;
    g_DefaultScreenHeight = 480;
}

// 0x004DF660, __thiscall (event, param). Every event in -8..0x40 except 0x15
// (Initialise) and 0x16 (Shutdown), whose callees are not decompiled yet.
ORACLE_TEST(GameApp_HandleEvent, "0x004DF660")
{
    auto original = At<HandleEventFn>(0x004DF660);
    Rng rng(0x4DF660);
    for (int round = 0; round < 50; ++round) {
        for (int event = -8; event <= 0x40; ++event) {
            if (event == kEvent_Initialise || event == kEvent_Shutdown)
                continue;
            alignas(8) uint8_t orig[kAppSize];
            alignas(8) uint8_t ours[kAppSize];
            Fill(orig, ours, rng);
            void* param = reinterpret_cast<void*>(static_cast<uintptr_t>(rng.Next()));
            const int ro = original(orig, nullptr, event, param);
            const int rd = reinterpret_cast<GameApp*>(ours)->HandleEvent(event, param);
            const std::string label = "event " + std::to_string(event);
            Compare(ro, rd, label + " return");
            CompareObjects(orig, ours, label);
        }
    }
    // Large and negative event numbers outside the table.
    for (int i = 0; i < 100000; ++i) {
        const int event = static_cast<int>(rng.Next());
        if (event == kEvent_Initialise || event == kEvent_Shutdown)
            continue;
        alignas(8) uint8_t orig[kAppSize];
        alignas(8) uint8_t ours[kAppSize];
        Fill(orig, ours, rng);
        Compare(original(orig, nullptr, event, nullptr),
                reinterpret_cast<GameApp*>(ours)->HandleEvent(event, nullptr),
                "event " + std::to_string(event));
    }
}

// 0x00424990, __cdecl; writes the 12-byte buffer at 0x006EBF80.
ORACLE_TEST(SetVersionString, "0x00424990")
{
    auto original = At<void(__cdecl*)(const char*)>(0x00424990);
    Rng rng(0x424990);
    for (int i = 0; i < 20000; ++i) {
        // Same starting bytes in both buffers, then a random string 0..20 chars.
        for (int k = 0; k < 12; ++k)
            OriginalVersionBuffer()[k] = g_VersionString[k] = static_cast<char>(rng.Next());
        char text[21];
        const int len = static_cast<int>(rng.Next() % 21);
        for (int k = 0; k < len; ++k)
            text[k] = static_cast<char>('!' + rng.Next() % 90);
        text[len] = '\0';
        original(text);
        SetVersionString(text);
        for (int k = 0; k < 12; ++k)
            Compare(OriginalVersionBuffer()[k], g_VersionString[k],
                    "\"" + std::string(text) + "\" byte " + std::to_string(k));
    }
}

// 0x004202E6, reached through its thunk 0x004DF730 (the body jumps back into
// the thunk's ret, so it must be called the way the game calls it).
ORACLE_TEST(AlwaysTrue, "0x004202E6")
{
    auto original = At<int(__cdecl*)()>(0x004DF730);
    for (int i = 0; i < 1000; ++i)
        Compare(original(), AlwaysTrue(), "call " + std::to_string(i));
}
