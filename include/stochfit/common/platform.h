#pragma once

// Cross-platform portability layer for StochFit.
// Provides: BOOL/TRUE/FALSE compatibility (int32_t), M_PI via <numbers>,
// MAX_OMP_THREADS limit, EXPORT visibility macro (dllexport / visibility("default")),
// platform_error() stderr sink, MyComplex alias (std::complex<double>),
// and STOCHFIT_HAS_GPU flag (set if STOCHFIT_HAS_CUDA or STOCHFIT_HAS_METAL).
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

// ── Complex number type ──────────────────────────────────────────────────────
using MyComplex = std::complex<double>;

// ── GPU availability ────────────────────────────────────────────────────────
enum class GpuBackend : int { None, CUDA, Metal };

#if defined(STOCHFIT_HAS_CUDA) || defined(STOCHFIT_HAS_METAL)
#  define STOCHFIT_HAS_GPU 1
#else
#  define STOCHFIT_HAS_GPU 0
#endif
