// Align.h — size alignment helpers.
//
// Decompiled from Manhunt2.exe (build 0x4992F67C).
//   AlignUp64  0x0060DA60  (25 B)  verified (tests/oracle/test_core.cpp)
#pragma once

#include <cstdint>

// Rounds `size` up to the next multiple of 64. 32-bit arithmetic, like the
// original: values within 63 of 2^32 wrap to 0.
inline uint32_t AlignUp64(uint32_t size)
{
    return (size & ~63u) + ((size & 63u) ? 64u : 0u);
}
