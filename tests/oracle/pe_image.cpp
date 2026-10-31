// pe_image.cpp — see pe_image.h.
#include "pe_image.h"

#include <bcrypt.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <psapi.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "psapi.lib")

namespace oracle {
namespace {

void GetMappedFileNameW_Optional(void* base, wchar_t* out, DWORD size)
{
    if (!GetMappedFileNameW(GetCurrentProcess(), base, out, size))
        out[0] = 0;
}

// System DLLs whose real functions may run inside a test.
const char* const kAllowedDlls[] = {
    "KERNEL32.dll", "USER32.dll", "ADVAPI32.dll", "SHLWAPI.dll", "SHELL32.dll",
    "GDI32.dll", "ole32.dll", "VERSION.dll", "WINMM.dll",
};

struct SavedSection {
    uint8_t* addr;
    std::vector<uint8_t> bytes;
};
std::vector<SavedSection> g_saved;

// --- import traps ----------------------------------------------------------

std::vector<std::string> g_trapNames;

void __cdecl TrapHit(unsigned index)
{
    std::fprintf(stderr, "\nORACLE TRAP: the original called import %s, which tests "
                         "do not provide.\n", g_trapNames[index].c_str());
    std::fflush(stderr);
    ExitProcess(3);
}

// Each trap is a tiny stub: push <index>; call TrapHit. Generated at run time
// in executable memory, one per trapped import.
void* MakeTrap(unsigned index)
{
    static uint8_t* page = nullptr;
    static size_t used = 0;
    const size_t stubSize = 16;
    if (page == nullptr || used + stubSize > 0x10000) {
        page = static_cast<uint8_t*>(VirtualAlloc(nullptr, 0x10000, MEM_COMMIT | MEM_RESERVE,
                                                  PAGE_EXECUTE_READWRITE));
        used = 0;
    }
    uint8_t* p = page + used;
    used += stubSize;
    p[0] = 0x68;                                   // push imm32
    std::memcpy(p + 1, &index, 4);
    p[5] = 0xE8;                                   // call rel32
    const int32_t rel = static_cast<int32_t>(reinterpret_cast<uintptr_t>(&TrapHit) -
                                             reinterpret_cast<uintptr_t>(p + 10));
    std::memcpy(p + 6, &rel, 4);
    p[10] = 0xCC;                                  // never reached
    return p;
}

bool Allowed(const char* dll)
{
    for (const char* a : kAllowedDlls)
        if (_stricmp(a, dll) == 0)
            return true;
    return false;
}

// --- hashing ---------------------------------------------------------------

std::string Sha1Hex(const std::vector<uint8_t>& data)
{
    BCRYPT_ALG_HANDLE alg = nullptr;
    uint8_t digest[20] = {};
    BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA1_ALGORITHM, nullptr, 0);
    BCryptHash(alg, nullptr, 0, const_cast<uint8_t*>(data.data()),
               static_cast<ULONG>(data.size()), digest, sizeof(digest));
    BCryptCloseAlgorithmProvider(alg, 0);
    char hex[41];
    for (int i = 0; i < 20; ++i)
        std::snprintf(hex + i * 2, 3, "%02x", digest[i]);
    return hex;
}

}  // namespace

bool MapOriginal(const wchar_t* exePath, bool (*releaseRange)())
{
    // Read the whole file.
    HANDLE f = CreateFileW(exePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) {
        std::fprintf(stderr, "cannot open %ls\n", exePath);
        return false;
    }
    std::vector<uint8_t> file(GetFileSize(f, nullptr));
    DWORD got = 0;
    ReadFile(f, file.data(), static_cast<DWORD>(file.size()), &got, nullptr);
    CloseHandle(f);

    const std::string sha = Sha1Hex(file);
    if (sha != kReferenceSha1) {
        std::fprintf(stderr, "wrong executable: SHA-1 %s, expected %s\n", sha.c_str(), kReferenceSha1);
        return false;
    }

    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(file.data());
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS32*>(file.data() + dos->e_lfanew);
    if (nt->OptionalHeader.ImageBase != kImageBase) {
        std::fprintf(stderr, "unexpected image base\n");
        return false;
    }

    // Free the host image and take the range at once: any allocation in
    // between (even this function's own buffers) could land there.
    const SIZE_T imageSize = nt->OptionalHeader.SizeOfImage;
    if (!releaseRange()) {
        std::fprintf(stderr, "cannot release the host image at 0x%08X\n",
                     static_cast<unsigned>(kImageBase));
        return false;
    }
    auto* base = static_cast<uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(kImageBase), imageSize,
                                                    MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    if (base != reinterpret_cast<uint8_t*>(kImageBase)) {
        std::fprintf(stderr, "cannot reserve 0x%08X (error %lu). Already in use:\n",
                     static_cast<unsigned>(kImageBase), GetLastError());
        for (uintptr_t a = kImageBase; a < kImageBase + imageSize;) {
            MEMORY_BASIC_INFORMATION mbi = {};
            VirtualQuery(reinterpret_cast<void*>(a), &mbi, sizeof(mbi));
            if (mbi.State != MEM_FREE) {
                wchar_t name[MAX_PATH] = L"";
                GetMappedFileNameW_Optional(mbi.AllocationBase, name, MAX_PATH);
                std::fprintf(stderr, "  0x%08X +0x%08X  alloc 0x%08X  type 0x%lX  %ls\n",
                             static_cast<unsigned>(a), static_cast<unsigned>(mbi.RegionSize),
                             static_cast<unsigned>(reinterpret_cast<uintptr_t>(mbi.AllocationBase)),
                             mbi.Type, name);
            }
            a += mbi.RegionSize ? mbi.RegionSize : 0x1000;
        }
        return false;
    }

    // Headers and sections. VirtualAlloc memory is already zero, which covers BSS.
    std::memcpy(base, file.data(), nt->OptionalHeader.SizeOfHeaders);
    auto* sec = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        const DWORD raw = (std::min)(sec[i].SizeOfRawData, sec[i].Misc.VirtualSize);
        if (raw)
            std::memcpy(base + sec[i].VirtualAddress, file.data() + sec[i].PointerToRawData, raw);
    }

    // Imports.
    const auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    for (auto* imp = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress);
         imp->Name != 0; ++imp) {
        const char* dll = reinterpret_cast<const char*>(base + imp->Name);
        HMODULE mod = Allowed(dll) ? LoadLibraryA(dll) : nullptr;
        auto* names = reinterpret_cast<IMAGE_THUNK_DATA32*>(
            base + (imp->OriginalFirstThunk ? imp->OriginalFirstThunk : imp->FirstThunk));
        auto* iat = reinterpret_cast<IMAGE_THUNK_DATA32*>(base + imp->FirstThunk);
        for (; names->u1.AddressOfData != 0; ++names, ++iat) {
            std::string label = dll;
            FARPROC fn = nullptr;
            if (IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal)) {
                const WORD ord = IMAGE_ORDINAL32(names->u1.Ordinal);
                label += "!#" + std::to_string(ord);
                if (mod)
                    fn = GetProcAddress(mod, MAKEINTRESOURCEA(ord));
            } else {
                auto* byName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);
                label += std::string("!") + byName->Name;
                if (mod)
                    fn = GetProcAddress(mod, byName->Name);
            }
            if (fn == nullptr) {
                g_trapNames.push_back(label);
                fn = reinterpret_cast<FARPROC>(MakeTrap(static_cast<unsigned>(g_trapNames.size() - 1)));
            }
            iat->u1.Function = reinterpret_cast<DWORD>(fn);
        }
    }

    // Remember the writable sections for RestoreData().
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        if (sec[i].Characteristics & IMAGE_SCN_MEM_WRITE)
            g_saved.push_back({base + sec[i].VirtualAddress, {}});
    }
    for (unsigned i = 0, k = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        if (sec[i].Characteristics & IMAGE_SCN_MEM_WRITE)
            g_saved[k++].bytes.resize(sec[i].Misc.VirtualSize);
    }
    SnapshotData();
    FlushInstructionCache(GetCurrentProcess(), base, imageSize);
    return true;
}

void SnapshotData()
{
    for (auto& s : g_saved)
        std::memcpy(s.bytes.data(), s.addr, s.bytes.size());
}

void RestoreData()
{
    for (auto& s : g_saved)
        std::memcpy(s.addr, s.bytes.data(), s.bytes.size());
}

}  // namespace oracle
