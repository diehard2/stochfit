#pragma once

// Cross-platform portability layer for StochFit.
// Provides: MAX_OMP_THREADS limit, EXPORT visibility macro (dllexport / visibility("default")),
// and platform_error() stderr sink.
// Use std::numbers::pi for π and std::complex<double> directly in all code.
// All standard headers needed by the codebase are included here.

// Cross-platform replacement for stdafx.h — all Windows-specific constructs
// are either stubbed out or replaced with C++23 equivalents.

// ── Standard C/C++ includes ────────────────────────────────────────────────
#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <tl/expected.hpp>

// std::jthread is NOT used — replaced with std::thread + std::atomic<bool>
// stop flag for cross-platform portability (Apple libc++ support varies).

// ── OpenMP ──────────────────────────────────────────────────────────────────
#include <omp.h>

// ── OMP thread limit ────────────────────────────────────────────────────────
#ifndef MAX_OMP_THREADS
    #define MAX_OMP_THREADS 8
#endif

// ── OMP wait policy (non-Windows) ───────────────────────────────────────────
// MSVC's vcomp runtime busy-spins at barriers by default. libgomp/libomp
// default to a short spin followed by a futex sleep, and the wake latency for
// that sleep is much higher inside a virtualized/WSL2 scheduler. The SA loop
// in StochFitHarness::Processing() hits an omp barrier every iteration, so a
// passive wait there serializes the whole run behind futex wakeups instead of
// keeping cores busy — this showed up as ~22% CPU (vs. ~80% on Windows) and a
// matching multi-x slowdown under WSL.
//
// NOTE: this setenv() is NOT sufficient on its own when libstochfit is loaded
// as a shared library (the Electron/koffi path). libgomp reads these vars in
// its own loader constructor, and because libgomp.so is a NEEDED dependency
// of libstochfit.so, the ELF loader always runs libgomp's constructor first —
// before this translation unit's static initializer ever gets to call
// setenv(). By then libgomp has already cached the passive policy. The
// authoritative fix for the GUI lives in gui/src/main/native/ffi.ts
// (ensureOmpWaitPolicy), which sets the vars via Node's process.env *before*
// koffi.load() triggers the dlopen() that pulls in libgomp.so. This copy is
// kept as a best-effort default for statically-linked consumers (mirefl,
// stochfit_tests) where ordering may differ, but is not guaranteed to win the
// race against libgomp's constructor in the general case.
#if !defined(_WIN32)
    #include <cstdlib>
    namespace stochfit_detail {
    inline void SetActiveOmpWaitPolicy()
    {
        setenv("OMP_WAIT_POLICY", "active", 0);
        setenv("GOMP_SPINCOUNT", "30000000", 0);
    }
    struct OmpWaitPolicyInit {
        OmpWaitPolicyInit() { SetActiveOmpWaitPolicy(); }
    };
    static OmpWaitPolicyInit g_ompWaitPolicyInit;
    }  // namespace stochfit_detail
#endif

// ── Debug flags ──────────────────────────────────────────────────────────────
inline constexpr bool kSingleProcDebug = false;

// ── Export macro ────────────────────────────────────────────────────────────
#if defined(_MSC_VER)
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

// ── MessageBox replacement ──────────────────────────────────────────────────
inline void platform_error(const char* msg)
{
    std::cerr << msg << std::endl;
}

// ── Convenience namespaces (matching existing codebase convention) ──────────
using namespace std;
