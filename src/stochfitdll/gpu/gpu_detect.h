#pragma once

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
