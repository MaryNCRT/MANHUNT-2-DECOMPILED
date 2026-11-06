// test_capp.cpp — differential tests for decomp/core/CApp.cpp.
#include "oracle.h"
#include "pe_image.h"

#include "../../decomp/core/CApp.h"

#include <cstring>
#include <memory>
#include <string>

using namespace oracle;

// 0x004B0570, __cdecl; stores its argument at 0x006ECBA4.
ORACLE_TEST(SetLoadingMessage, "0x004B0570")
{
    auto original = At<void(__cdecl*)(const char*)>(0x004B0570);
    auto& stored = *At<const char**>(0x006ECBA4);
    Rng rng(0x4B0570);
    for (int i = 0; i < 100000; ++i) {
        const char* p = reinterpret_cast<const char*>(static_cast<uintptr_t>(rng.Next()));
        original(p);
        SetLoadingMessage(p);
        Compare(reinterpret_cast<uintptr_t>(stored), reinterpret_cast<uintptr_t>(g_LoadingMessage),
                "pointer " + std::to_string(reinterpret_cast<uintptr_t>(p)));
    }
}

// 0x004B08C0, __thiscall constructor; must zero exactly what the original
// zeroes, leave the rest alone, and return this.
ORACLE_TEST(Vec4Table_F620_Construct, "0x004B08C0")
{
    using CtorFn = void*(__fastcall*)(void* self, void* edx);
    auto original = At<CtorFn>(0x004B08C0);
    constexpr size_t n = sizeof(Vec4Table_F620);
    auto orig = std::make_unique<uint8_t[]>(n);
    auto ours = std::make_unique<uint8_t[]>(n);
    Rng rng(0x4B08C0);
    for (int round = 0; round < 20; ++round) {
        for (size_t i = 0; i < n; ++i)
            orig[i] = ours[i] = static_cast<uint8_t>(rng.Next());
        void* ro = original(orig.get(), nullptr);
        void* rd = reinterpret_cast<Vec4Table_F620*>(ours.get())->Construct();
        Compare(ro == orig.get() ? 1 : 0, rd == ours.get() ? 1 : 0, "returns this");
        for (size_t i = 0; i < n; ++i)
            Compare(orig[i], ours[i], "byte +" + std::to_string(i));
    }
}

// 0x0056EFB0, __cdecl; zeroes 36 dwords at 0x0076BD80, a dword at 0x0076BE40
// and a byte at 0x0076BE50. The bytes around them must stay as they were.
ORACLE_TEST(ResetGlobals_0076BD80, "0x0056EFB0")
{
    auto original = At<void(__cdecl*)()>(0x0056EFB0);
    uint8_t* region = At<uint8_t*>(0x0076BD80);
    constexpr size_t kRegion = 0x0076BE60 - 0x0076BD80;
    Rng rng(0x56EFB0);
    for (int round = 0; round < 1000; ++round) {
        uint8_t before[kRegion];
        for (size_t i = 0; i < kRegion; ++i)
            region[i] = before[i] = static_cast<uint8_t>(rng.Next());
        std::memcpy(g_Globals_0076BD80.words, before, sizeof(g_Globals_0076BD80.words));
        std::memcpy(&g_Globals_0076BD80.at0076BE40, before + 0xC0, 4);
        g_Globals_0076BD80.at0076BE50 = before[0xD0];

        original();
        ResetGlobals_0076BD80();

        for (int w = 0; w < 36; ++w) {
            int32_t o;
            std::memcpy(&o, region + w * 4, 4);
            Compare(o, g_Globals_0076BD80.words[w], "word " + std::to_string(w));
        }
        int32_t o40;
        std::memcpy(&o40, region + 0xC0, 4);
        Compare(o40, g_Globals_0076BD80.at0076BE40, "0x76BE40");
        Compare(region[0xD0], g_Globals_0076BD80.at0076BE50, "0x76BE50");
        // Untouched by the original: everything that is not one of the fields.
        for (size_t i = 0x90; i < kRegion; ++i) {
            if ((i >= 0xC0 && i < 0xC4) || i == 0xD0)
                continue;
            Compare(region[i], before[i], "untouched +" + std::to_string(i));
        }
    }
}
