// host.cpp — placeholder executable that owns the address range of the original.
//
// Windows maps a process's own image before creating its heaps and NLS views,
// and those land right where Manhunt2.exe must go (0x00400000-0x00BD9000). So
// this tiny program is linked at 0x00400000 with an image large enough to
// cover that range. Its only job is to load oracle.dll and jump into it; the
// DLL then unmaps this image and maps the original in its place. Control never
// comes back here.
//
// Built without the CRT (/NODEFAULTLIB, /ENTRY:HostEntry).
#include <windows.h>

// Makes SizeOfImage cover the original's 0x7D9000 bytes. Uninitialized, so
// the file stays small.
#pragma bss_seg(".reserve")
static char g_reserve[0x7E0000];
#pragma bss_seg()

extern "C" void __stdcall HostEntry()
{
    g_reserve[0] = 0;  // keep the section referenced
    HMODULE dll = LoadLibraryW(L"oracle.dll");
    if (dll == nullptr)
        ExitProcess(4);
    auto oracleMain = reinterpret_cast<void(__cdecl*)()>(GetProcAddress(dll, "OracleMain"));
    if (oracleMain == nullptr)
        ExitProcess(4);
    oracleMain();  // does not return
    ExitProcess(4);
}
