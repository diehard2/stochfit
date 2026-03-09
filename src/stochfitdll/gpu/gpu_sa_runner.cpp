#include "gpu_sa_runner.h"

#if defined(STOCHFIT_HAS_CUDA)
#include "stochfit_cuda.h"
#endif

#if defined(STOCHFIT_HAS_METAL)
#include "stochfit_metal.h"
#endif

std::unique_ptr<GpuSARunner> GpuSARunner::create(GpuBackend backend)
{
#if defined(STOCHFIT_HAS_CUDA)
    if (backend == GpuBackend::CUDA)
        return create_cuda_runner();
#endif

#if defined(STOCHFIT_HAS_METAL)
    if (backend == GpuBackend::Metal)
        return create_metal_runner();
#endif

    return nullptr;
}
