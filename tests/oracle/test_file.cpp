// test_file.cpp — differential tests for decomp/io/File.cpp.
#include "oracle.h"
#include "pe_image.h"

#include "../../decomp/io/File.h"

#include <cstring>
#include <string>
#include <vector>

using namespace oracle;

namespace {

// A byte stream both sides read from. One instance per side and case, with
// identical contents, so consumption can be compared too.
struct Stream {
    std::vector<char> bytes;
    size_t at = 0;
    bool eof() const { return at >= bytes.size(); }
    int read(void* dst, int n)
    {
        int k = 0;
        while (k < n && !eof())
            static_cast<char*>(dst)[k++] = bytes[at++];
        return k;
    }
};

// Our side: a File whose IsEOF/Read come from a Stream.
class StreamFile final : public File {
public:
    explicit StreamFile(Stream* s) : s_(s) {}
    bool IsEOF() override { return s_->eof(); }
    uint32_t GetSize() override { return static_cast<uint32_t>(s_->bytes.size()); }
    int Read(void* dst, int n) override { return s_->read(dst, n); }
    void Seek(int) override {}
    void Close() override {}
private:
    Stream* s_;
};

// The original's side: an object laid out like the original's File, whose
// vtable slots +0x04 (IsEOF) and +0x0C (Read) are these __thiscall-compatible
// functions (__fastcall: this in ECX, EDX unused, callee pops the stack args).
struct FakeObject {
    void** vtable;
    Stream* stream;
};
bool __fastcall FakeIsEOF(FakeObject* self, void*) { return self->stream->eof(); }
int __fastcall FakeRead(FakeObject* self, void*, void* dst, int n) { return self->stream->read(dst, n); }
void __fastcall FakeUnreachable(FakeObject*, void*)
{
    std::fprintf(stderr, "\nORACLE: unexpected vtable slot called\n");
    std::exit(3);
}
void* g_fakeVtable[7] = {
    reinterpret_cast<void*>(&FakeUnreachable), reinterpret_cast<void*>(&FakeIsEOF),
    reinterpret_cast<void*>(&FakeUnreachable), reinterpret_cast<void*>(&FakeRead),
    reinterpret_cast<void*>(&FakeUnreachable), reinterpret_cast<void*>(&FakeUnreachable),
    reinterpret_cast<void*>(&FakeUnreachable),
};

std::vector<char> RandomText(Rng& rng)
{
    static const char kAlphabet[] = "ab# \"\r\n\n\0x";
    std::vector<char> v(rng.Next() % 48);
    for (char& c : v)
        c = kAlphabet[rng.Next() % (sizeof(kAlphabet) - 1)];
    return v;
}

}  // namespace

// 0x0053ABE0, __thiscall (dst, max), ret 8.
ORACLE_TEST(File_ReadLine, "0x0053ABE0")
{
    using ReadLineFn = char*(__fastcall*)(void* self, void* edx, char* dst, int max);
    auto original = At<ReadLineFn>(0x0053ABE0);
    Rng rng(0x53ABE0);
    for (int i = 0; i < 200000; ++i) {
        const std::vector<char> text = RandomText(rng);
        const int max = static_cast<int>(rng.Next() % 24) - 2;  // includes 0 and negatives
        char bufO[64], bufD[64];
        for (int k = 0; k < 64; ++k)
            bufO[k] = bufD[k] = static_cast<char>(0x40 + rng.Next() % 26);

        Stream so{text}, sd{text};
        FakeObject fake{g_fakeVtable, &so};
        StreamFile ours(&sd);
        char* ro = original(&fake, nullptr, bufO, max);
        char* rd = ours.ReadLine(bufD, max);

        const std::string label = "case " + std::to_string(i) + " max " + std::to_string(max);
        Compare(ro == nullptr ? 0 : (ro == bufO ? 1 : 2), rd == nullptr ? 0 : (rd == bufD ? 1 : 2),
                label + " return");
        Compare(so.at, sd.at, label + " bytes consumed");
        for (int k = 0; k < 64; ++k)
            Compare(bufO[k], bufD[k], label + " byte " + std::to_string(k));
    }
}

// 0x0053ACF0 / 0x0053AD10, __thiscall field reads on the packed reader.
ORACLE_TEST(PackedFile_IsEOF_GetSize, "0x0053ACF0, 0x0053AD10")
{
    using BoolFn = bool(__fastcall*)(void* self, void* edx);
    using SizeFn = uint32_t(__fastcall*)(void* self, void* edx);
    auto origEOF = At<BoolFn>(0x0053ACF0);
    auto origSize = At<SizeFn>(0x0053AD10);
    static PackedFile ours;
    static uint8_t orig[sizeof(PackedFile)];
    Rng rng(0x53ACF0);
    for (int i = 0; i < 200000; ++i) {
        uint32_t size = rng.Next(), pos = rng.Next();
        if (i % 4 == 0) pos = size;                       // exactly at the end
        if (i % 4 == 1) pos = size ? size - 1 : 0;        // one before
        std::memcpy(orig + 0x2008, &size, 4);
        std::memcpy(orig + 0x200C, &pos, 4);
        ours.size = size;
        ours.position = pos;
        const std::string label = "pos " + std::to_string(pos) + " size " + std::to_string(size);
        Compare(origEOF(orig, nullptr) ? 1 : 0, ours.IsEOF() ? 1 : 0, label + " IsEOF");
        Compare(origSize(orig, nullptr), ours.GetSize(), label + " GetSize");
    }
}
