#include "gpu_detect.h"
#include <algorithm>

#if defined(STOCHFIT_HAS_CUDA)
#include <cuda_runtime.h>
#endif

#if defined(STOCHFIT_HAS_METAL)
// Metal detection is in stochfit_metal.mm
GpuInfo detect_metal_gpu();
#endif

GpuInfo detect_gpu()
{
    GpuInfo info;

#if defined(STOCHFIT_HAS_CUDA)
    int device_count = 0;
    cudaError_t err = cudaGetDeviceCount(&device_count);
    if (err == cudaSuccess && device_count > 0) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, 0);

        // Require compute capability >= 7.5 (Turing+)
        if (prop.major > 7 || (prop.major == 7 && prop.minor >= 5)) {
            info.backend = GpuBackend::CUDA;
            info.device_name = prop.name;
            info.memory_bytes = prop.totalGlobalMem;
            info.compute_capability_major = prop.major;
            info.compute_capability_minor = prop.minor;
            info.sm_count = prop.multiProcessorCount;
            // Heuristic: 2 chains per SM, capped at 128
            info.max_chains = std::min(prop.multiProcessorCount * 2, 128);
            return info;
        }
    }
#endif

#if defined(STOCHFIT_HAS_METAL)
    info = detect_metal_gpu();
    if (info.backend == GpuBackend::Metal)
        return info;
#endif

    return info;
}

bool is_gpu_available()
{
    return detect_gpu().backend != GpuBackend::None;
}
