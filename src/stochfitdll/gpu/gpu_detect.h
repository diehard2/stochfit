#pragma once

// GPU detection: probes for CUDA (compute 7.5+ / RTX 20+) or Metal (Apple Silicon).
// detect_gpu() returns a GpuInfo struct with backend, device name, SM count,
// available memory, and max_chains recommendation for multi-chain SA.
// is_gpu_available() is a convenience wrapper (backend != None).
// Used by StochFitHarness to gate ProcessingGPU() and by the GpuAvailable()
// FFI export queried by the frontend on startup.

#include <string>
#include <cstddef>

enum class GpuBackend : int { None, CUDA, Metal };

struct GpuInfo {
    GpuBackend backend = GpuBackend::None;
    std::string device_name;
    size_t memory_bytes = 0;
    int compute_capability_major = 0;
    int compute_capability_minor = 0;
    int sm_count = 0;
    int max_chains = 0;
};

GpuInfo detect_gpu();
bool is_gpu_available();
