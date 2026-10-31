// test_core.cpp — differential tests for decomp/core.
#include "oracle.h"
#include "pe_image.h"

#include "../../decomp/core/Align.h"
#include "../../decomp/core/ProcessCheck.h"

#include <string>

using namespace oracle;

// 0x0060DA60, __cdecl, pure.
ORACLE_TEST(AlignUp64, "0x0060DA60")
{
    auto original = At<uint32_t(__cdecl*)(uint32_t)>(0x0060DA60);

    // Every value around each multiple of 64 in the low range, both 32-bit
    // ends, then random values.
    for (uint32_t v = 0; v < 0x10000; ++v)
        Compare(original(v), AlignUp64(v), std::to_string(v));
    for (uint32_t v = 0xFFFF0000u; v != 0; ++v)
        Compare(original(v), AlignUp64(v), std::to_string(v));
    Rng rng(0x6DA60);
    for (int i = 0; i < 1000000; ++i) {
        const uint32_t v = rng.Next();
        Compare(original(v), AlignUp64(v), std::to_string(v));
    }
}

// 0x00615420, __cdecl. Reads the live process list, which other programs
// change at any moment, so it uses CompareLive. Names are chosen to give 0, 1
// and many matches.
ORACLE_TEST(CountProcessesNamed, "0x00615420")
{
    auto original = At<unsigned(__cdecl*)(const wchar_t*)>(0x00615420);

    const wchar_t* names[] = {
        nullptr, L"", L"oracle", L"svchost", L"System", L".exe", L"e",
        L"Manhunt2", L"MANHUNT2", L"no-such-process-name", L"explorer.exe",
    };
    for (int round = 0; round < 20; ++round) {
        for (const wchar_t* n : names) {
            char label[64];
            std::snprintf(label, sizeof(label), "\"%ls\"", n ? n : L"(null)");
            CompareLive([&] { return original(n); }, [&] { return CountProcessesNamed(n); }, label);
        }
    }
}
