// File.cpp — file interface helpers and the packed (.glg) reader.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   File::ReadLine         0x0053ABE0  (103 B)  verified (tests/oracle/test_file.cpp)
//   PackedFile::IsEOF      0x0053ACF0  ( 18 B)  verified
//   PackedFile::GetSize    0x0053AD10  (  7 B)  verified
//   PackedFile::Read       0x0053AD20  (330 B)  readable
//   PackedFile::Close      0x0053AE70  ( 37 B)  readable
//   File_Open              0x0053BD00  (190 B)  readable

#include "File.h"

#include <cstring>

// zlib 1.1.4, linked into the original (inflateInit_ 0x006117A0,
// inflate 0x006117C0, inflateEnd 0x00611650). Library code, not decompiled.
extern "C" int Zlib_inflateInit_(ZStream114* s, const char* version, int streamSize);
extern "C" int Zlib_inflate(ZStream114* s, int flush);
extern "C" int Zlib_inflateEnd(ZStream114* s);

File* RawFile_Open(const char* path, int mode);   // 0x0053BCC0
extern PackedFile g_PackedFile;                   // 0x006A0390; its `raw` field is 0x006A0394
void* Zlib_Alloc(void*, unsigned, unsigned);      // 0x0053AEA0
void  Zlib_Free(void*, void*);                    // 0x0053AEC0

static const uint32_t kPackedMagic = 0x4D48325A;  // "Z2HM" read as a little-endian dword

char* File::ReadLine(char* dst, int max)
{
    if (max == 0 || IsEOF())
        return nullptr;
    char* p = dst;
    for (;;) {
        char c = 0;
        Read(&c, 1);
        if (c != '\r') {
            // Every character is stored; the cursor only advances while room
            // is left, so a long line keeps overwriting its last slot.
            --max;
            *p = c;
            if (max > 0)
                ++p;
        }
        if (c == '\0' || c == '\n')
            break;
    }
    *p = '\0';  // the '\n' stays in the line when it fits
    return dst;
}

bool PackedFile::IsEOF()
{
    return position >= size;
}

uint32_t PackedFile::GetSize()
{
    return size;
}

int PackedFile::Read(void* dst, int n)
{
    uint8_t* to = static_cast<uint8_t*>(dst);
    int done = 0;
    while (n > 0) {
        if (pending == 0) {
            // Refill `out` with up to 4 KB of unpacked data.
            if (stream.avail_in == 0 && raw->IsEOF())
                break;
            stream.next_out = out;
            stream.avail_out = sizeof(out);
            while (true) {
                if (stream.avail_in == 0) {
                    if (raw->IsEOF())
                        break;
                    if (stream.avail_in == 0) {
                        stream.next_in = in;
                        stream.avail_in = raw->Read(in, sizeof(in));
                    }
                }
                Zlib_inflate(&stream, 0);
                if (stream.avail_out == 0)
                    break;
            }
            pending = sizeof(out) - stream.avail_out;
            pendingAt = out;
        }
        int chunk = static_cast<int>(pending);
        if (n <= chunk)
            chunk = n;
        if (chunk == 1) {
            *to = *pendingAt;
        } else {
            std::memcpy(to, pendingAt, chunk);
        }
        done += chunk;
        position += chunk;
        pendingAt += chunk;
        pending -= chunk;
        to += chunk;
        n -= chunk;
    }
    return done;
}

void PackedFile::Close()
{
    Zlib_inflateEnd(&stream);
    raw->Close();
    raw = nullptr;
}

File* File_Open(const char* path)
{
    // The raw file goes straight into the global reader's `raw` field
    // (0x006A0394 = 0x006A0390 + 4).
    g_PackedFile.raw = RawFile_Open(path, 1);
    if (g_PackedFile.raw == nullptr)
        return nullptr;

    // PORT: initialized. The original reads into an uninitialized local, so a
    // file shorter than 4 bytes is compared against stack garbage.
    uint32_t magic = 0;
    g_PackedFile.raw->Read(&magic, 4);
    if (magic != kPackedMagic) {
        // Not packed: rewind, detach from the reader, hand back the plain file.
        File* plain = g_PackedFile.raw;
        plain->Seek(0);
        g_PackedFile.raw = nullptr;
        return plain;
    }

    uint32_t unpacked = 0;
    g_PackedFile.raw->Read(&unpacked, 4);
    // ORIGINAL LIMIT L4: always the same global object. Opening a second
    // packed file resets the first one's stream.
    g_PackedFile.size = unpacked;
    g_PackedFile.position = 0;
    g_PackedFile.stream.zalloc = reinterpret_cast<void*>(&Zlib_Alloc);
    g_PackedFile.stream.zfree = reinterpret_cast<void*>(&Zlib_Free);
    g_PackedFile.stream.opaque = nullptr;
    g_PackedFile.stream.next_in = nullptr;
    g_PackedFile.stream.avail_in = 0;
    Zlib_inflateInit_(&g_PackedFile.stream, "1.1.4", sizeof(ZStream114));
    return &g_PackedFile;
}
