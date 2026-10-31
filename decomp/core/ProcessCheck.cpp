// ProcessCheck.cpp — single-instance check used by WinMain.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   CountProcessesNamed  0x00615420  (197 B)  verified (tests/oracle/test_core.cpp)

#include "ProcessCheck.h"

#include <tlhelp32.h>
#include <cwchar>

// Case-sensitive substring match, like the original's wcsstr call.
// Returns 0 if `name` is null or the snapshot fails.
unsigned CountProcessesNamed(const WCHAR* name)
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
