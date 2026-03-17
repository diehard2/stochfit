#pragma once

// Cross-platform portability layer for StochFit.
// Provides: MAX_OMP_THREADS limit, EXPORT visibility macro (dllexport / visibility("default")),
// platform_error() stderr sink, and STOCHFIT_HAS_GPU flag (set if STOCHFIT_HAS_CUDA or STOCHFIT_HAS_METAL).
// Use std::numbers::pi for π and std::complex<double> directly in all code.
// All standard headers needed by the codebase are included here.

// Cross-platform replacement for stdafx.h — all Windows-specific constructs
// are either stubbed out or replaced with C++23 equivalents.

// ── Standard C/C++ includes ────────────────────────────────────────────────
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <climits>
#include <complex>
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
#include <random>
#include <optional>
#include <tl/expected.hpp>

// std::jthread is NOT used — replaced with std::thread + std::atomic<bool>
// stop flag for cross-platform portability (Apple libc++ support varies).

// ── OpenMP ──────────────────────────────────────────────────────────────────
#include <omp.h>

// ── OMP thread limit ────────────────────────────────────────────────────────
#ifndef MAX_OMP_THREADS
#  define MAX_OMP_THREADS 8
#endif

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

// ── GPU availability ────────────────────────────────────────────────────────
enum class GpuBackend : int { None, CUDA, Metal };

#if defined(STOCHFIT_HAS_CUDA) || defined(STOCHFIT_HAS_METAL)
#  define STOCHFIT_HAS_GPU 1
#else
#  define STOCHFIT_HAS_GPU 0
#endif
