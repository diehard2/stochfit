#pragma once

// Cross-platform replacement for stdafx.h — all Windows-specific constructs
// are either stubbed out or replaced with C++23 equivalents.

// ── Standard C/C++ includes ────────────────────────────────────────────────
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <climits>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numbers>
#include <thread>
#include <atomic>
#include <chrono>

// ── OpenMP ──────────────────────────────────────────────────────────────────
#ifdef _OPENMP
#  include <omp.h>
#endif

// ── BOOL compatibility ──────────────────────────────────────────────────────
using BOOL = int32_t;
#ifndef TRUE
#  define TRUE  1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

// ── Math constants ──────────────────────────────────────────────────────────
#ifndef M_PI
#  define M_PI std::numbers::pi
#endif

// ── OMP thread limit ────────────────────────────────────────────────────────
#ifndef MAX_OMP_THREADS
#  define MAX_OMP_THREADS 8
#endif

// ── Aligned alloc wrappers ──────────────────────────────────────────────────
// Signature matches _mm_malloc / _aligned_malloc: (size, alignment).
inline void* platform_aligned_alloc(size_t size, size_t alignment) {
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    // std::aligned_alloc requires size to be a multiple of alignment
    size_t rounded = ((size + alignment - 1u) / alignment) * alignment;
    return std::aligned_alloc(alignment, rounded);
#endif
}

inline void platform_aligned_free(void* p) {
#if defined(_MSC_VER)
    _aligned_free(p);
#else
    std::free(p);
#endif
}

// ── Export macro ────────────────────────────────────────────────────────────
#if defined(_MSC_VER)
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __attribute__((visibility("default")))
#endif

// ── MessageBox replacement ──────────────────────────────────────────────────
inline void platform_error(const char* msg) { std::cerr << msg << std::endl; }

// ── Convenience namespaces (matching existing codebase convention) ──────────
using namespace std;

// ── Project headers ─────────────────────────────────────────────────────────
#include <stochfit/common/MyComplex.h>
#include <stochfit/common/random.h>

using namespace MyComplexNumber;
using namespace Random;
