// oracle.h — minimal differential test runner.
//
// A test calls the original function inside the mapped Manhunt2.exe and our
// decompiled version with the same inputs, and records every divergence.
// Acceptance criterion: zero divergences.
#pragma once

#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace oracle {

struct Test {
    const char* name;
    const char* original;   // address of the original function, as text
    std::function<void()> body;
};

std::vector<Test>& Registry();

struct Register {
    Register(const char* name, const char* original, std::function<void()> body)
    {
        Registry().push_back({name, original, std::move(body)});
    }
};

// Per-test counters, reset by the runner.
extern uint64_t g_cases;
extern uint64_t g_divergences;
extern uint64_t g_unstable;  // cases skipped because the input changed under the test

// Records one comparison. Prints the first few divergences of a test.
template <typename T>
void Compare(const T& original, const T& decomp, const std::string& input)
{
    ++g_cases;
    if (original == decomp)
        return;
    if (++g_divergences <= 10)
        std::printf("    DIVERGENCE on %s: original %llu, decomp %llu\n", input.c_str(),
                    static_cast<unsigned long long>(original),
                    static_cast<unsigned long long>(decomp));
}

// For functions that read live system state (process list, clock...): the
// original runs before and after ours. If its two results differ, the state
// changed mid-case and the case is skipped and counted as unstable, never as a
// pass. Otherwise ours must match.
template <typename Orig, typename Decomp>
void CompareLive(Orig original, Decomp decomp, const std::string& input)
{
    const auto before = original();
    const auto ours = decomp();
    const auto after = original();
    if (before != after) {
        ++g_unstable;
        return;
    }
    Compare(before, ours, input);
}

// Deterministic generator, so a failing run can be reproduced exactly.
struct Rng {
    uint64_t s;
    explicit Rng(uint64_t seed) : s(seed ? seed : 0x9E3779B97F4A7C15ull) {}
    uint32_t Next()
    {
        s ^= s << 13;
        s ^= s >> 7;
        s ^= s << 17;
        return static_cast<uint32_t>(s >> 16);
    }
};

}  // namespace oracle

#define ORACLE_CONCAT2(a, b) a##b
#define ORACLE_CONCAT(a, b) ORACLE_CONCAT2(a, b)
// The test body gets a "test_" prefix so a test can share its name with the
// decompiled function it checks.
#define ORACLE_TEST(name, original) \
    static void ORACLE_CONCAT(test_, name)(); \
    static oracle::Register ORACLE_CONCAT(reg_, name)(#name, original, ORACLE_CONCAT(test_, name)); \
    static void ORACLE_CONCAT(test_, name)()
