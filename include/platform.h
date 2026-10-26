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
