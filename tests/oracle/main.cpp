// main.cpp — oracle test runner (lives in oracle.dll, started by host.exe).
//
//   oracle_host.exe <path to Manhunt2_nosecu.exe> [test name filter]
//
// Exit code 0 only if every test ran with zero divergences.
#include "oracle.h"
#include "pe_image.h"

#include <windows.h>
#include <shellapi.h>
#include <cstring>

#pragma comment(lib, "shell32.lib")

namespace oracle {
uint64_t g_cases = 0;
uint64_t g_divergences = 0;
uint64_t g_unstable = 0;
std::vector<Test>& Registry()
{
    static std::vector<Test> tests;
    return tests;
}
}  // namespace oracle

namespace {

// Frees the host image at 0x00400000 so the original can take its place.
bool ReleaseHostImage()
{
    using NtUnmapViewOfSection_t = LONG(NTAPI*)(HANDLE, PVOID);
    auto unmap = reinterpret_cast<NtUnmapViewOfSection_t>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtUnmapViewOfSection"));
    return unmap && unmap(GetCurrentProcess(), reinterpret_cast<PVOID>(oracle::kImageBase)) >= 0;
}

int Run(int argc, wchar_t** argv)
{
    if (argc < 2) {
        std::printf("usage: oracle_host <Manhunt2_nosecu.exe> [filter]\n");
        return 2;
    }
    if (!oracle::MapOriginal(argv[1], ReleaseHostImage))
        return 2;
    std::printf("original mapped at 0x%08X\n\n", static_cast<unsigned>(oracle::kImageBase));

    char filter[128] = {};
    if (argc > 2)
        std::snprintf(filter, sizeof(filter), "%ls", argv[2]);

    int failed = 0, ran = 0;
    for (auto& t : oracle::Registry()) {
        if (filter[0] && std::strstr(t.name, filter) == nullptr)
            continue;
        oracle::RestoreData();
        oracle::g_cases = oracle::g_divergences = oracle::g_unstable = 0;
        std::printf("%-36s vs %s\n", t.name, t.original);
        t.body();
        const bool ok = oracle::g_divergences == 0 && oracle::g_cases > 0;
        std::printf("    %s  %llu cases, %llu divergences", ok ? "PASS" : "FAIL",
                    static_cast<unsigned long long>(oracle::g_cases),
                    static_cast<unsigned long long>(oracle::g_divergences));
        if (oracle::g_unstable)
            std::printf(", %llu unstable (skipped)", static_cast<unsigned long long>(oracle::g_unstable));
        std::printf("\n");
        ++ran;
        failed += ok ? 0 : 1;
    }
    std::printf("\n%d tests, %d failed\n", ran, failed);
    return failed ? 1 : 0;
}

}  // namespace

extern "C" __declspec(dllexport) void __cdecl OracleMain()
{
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    const int code = Run(argc, argv);
    std::fflush(stdout);
    ExitProcess(static_cast<UINT>(code));
}
