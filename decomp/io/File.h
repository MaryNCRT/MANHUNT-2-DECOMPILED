// File.h — the engine's file interface and the packed (.glg) file reader.
//
// The virtual methods are declared in the original's slot order, so MSVC
// lays out the vtable the same way (slot 0 is the deleting destructor).
#pragma once

#include <cstddef>
#include <cstdint>

// Base class (vtable 0x00661CFC in the original).
class File {
public:
    virtual ~File() = default;                   // +0x00
    virtual bool     IsEOF() = 0;                // +0x04
    virtual uint32_t GetSize() = 0;              // +0x08
    virtual int      Read(void* dst, int n) = 0; // +0x0C  returns bytes read
    virtual char*    ReadLine(char* dst, int max);  // +0x10  0x0053ABE0
    virtual void     Seek(int offset) = 0;       // +0x14
    virtual void     Close() = 0;                // +0x18
};

// zlib 1.1.4 z_stream as the 32-bit original lays it out (0x38 bytes).
struct ZStream114 {
    const uint8_t* next_in;   // +0x00
    uint32_t       avail_in;  // +0x04
    uint32_t       total_in;  // +0x08
    uint8_t*       next_out;  // +0x0C
    uint32_t       avail_out; // +0x10
    uint32_t       total_out; // +0x14
    const char*    msg;       // +0x18
    void*          state;     // +0x1C
    void*          zalloc;    // +0x20  0x0053AEA0 in the original
    void*          zfree;     // +0x24  0x0053AEC0
    void*          opaque;    // +0x28
    int32_t        data_type; // +0x2C
    uint32_t       adler;     // +0x30
    uint32_t       reserved;  // +0x34
};

// Reader for "Z2HM" packed files (vtable 0x00661D3C). The original has ONE
// instance, a global at 0x006A0390, so only one packed file can be open at a
// time (limit L4).
class PackedFile final : public File {
public:
    bool     IsEOF() override;                   // 0x0053ACF0
    uint32_t GetSize() override;                 // 0x0053AD10
    int      Read(void* dst, int n) override;    // 0x0053AD20
    void     Seek(int) override {}               // 0x004670B0: "ret 4", packed files cannot seek
    void     Close() override;                   // 0x0053AE70

    File*      raw;                    // +0x0004  the file underneath
    uint8_t    in[0x1000];             // +0x0008  packed bytes
    uint8_t    out[0x1000];            // +0x1008  unpacked bytes
    uint32_t   size;                   // +0x2008  unpacked size, from the header
    uint32_t   position;               // +0x200C  bytes handed out
    uint32_t   pending;                // +0x2010  unread bytes in `out`
    uint8_t*   pendingAt;              // +0x2014
    ZStream114 stream;                 // +0x2018
};

#if defined(_M_IX86)
static_assert(offsetof(PackedFile, raw) == 0x0004, "PackedFile layout");
static_assert(offsetof(PackedFile, size) == 0x2008, "PackedFile layout");
static_assert(offsetof(PackedFile, stream) == 0x2018, "PackedFile layout");
static_assert(sizeof(ZStream114) == 0x38, "zlib 1.1.4 z_stream");
#endif

// 0x0053BD00 — opens `path`. Returns the raw file if it does not start with
// "Z2HM" (so loose text works wherever a .glg is expected), or the global
// PackedFile set up to inflate it. Null if the file cannot be opened.
File* File_Open(const char* path);
