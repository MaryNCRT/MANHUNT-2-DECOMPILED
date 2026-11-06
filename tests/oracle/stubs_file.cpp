// stubs_file.cpp — callees of decomp/io/File.cpp that are not decompiled yet
// (zlib, the raw file opener, the allocator hooks). The File tests never reach
// them; if one is reached the run stops instead of passing silently.
#include "../../decomp/io/File.h"

#include <cstdio>
#include <cstdlib>

[[noreturn]] static void Unreachable(const char* what)
{
    std::fprintf(stderr, "\nORACLE STUB: %s reached; it has no test stand-in.\n", what);
    std::exit(3);
}

extern "C" int Zlib_inflateInit_(ZStream114*, const char*, int) { Unreachable("inflateInit_"); }
extern "C" int Zlib_inflate(ZStream114*, int) { Unreachable("inflate"); }
extern "C" int Zlib_inflateEnd(ZStream114*) { Unreachable("inflateEnd"); }
File* RawFile_Open(const char*, int) { Unreachable("RawFile_Open"); }
void* Zlib_Alloc(void*, unsigned, unsigned) { Unreachable("Zlib_Alloc"); }
void Zlib_Free(void*, void*) { Unreachable("Zlib_Free"); }
PackedFile g_PackedFile;
