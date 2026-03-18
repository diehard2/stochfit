#include "gpu_detect.h"
#include <algorithm>
#include <cstdio>

// Platform-specific dynamic loading for CUDA driver detection.
// We probe nvcuda.dll (Windows) / libcuda.so (Linux) via LoadLibrary so that
// stochfit_core has NO hard dependency on any CUDA library.  stochfit.dll
// therefore loads cleanly on machines with no NVIDIA GPU driver installed.
#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
using LibHandle = HMODULE;
static LibHandle lib_open(const char* n)    { return LoadLibraryA(n); }
static void*     lib_sym(LibHandle h, const char* n) { return (void*)GetProcAddress(h, n); }
static void      lib_close(LibHandle h)     { FreeLibrary(h); }
#elif !defined(__APPLE__)
#  include <dlfcn.h>
using LibHandle = void*;
static LibHandle lib_open(const char* n)    { return dlopen(n, RTLD_LAZY | RTLD_LOCAL); }
static void*     lib_sym(LibHandle h, const char* n) { return dlsym(h, n); }
static void      lib_close(LibHandle h)     { dlclose(h); }
#endif

#if defined(STOCHFIT_HAS_METAL)
GpuInfo detect_metal_gpu();
#endif

// CUDA driver API minimal types and constants — avoids including cuda.h
typedef int CUresult;
typedef int CUdevice;
static constexpr CUresult CUDA_SUCCESS                                = 0;
static constexpr int CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT        = 16;
static constexpr int CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR    = 75;
static constexpr int CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR    = 76;

typedef CUresult (*PFN_cuInit)(unsigned int);
typedef CUresult (*PFN_cuDeviceGetCount)(int*);
typedef CUresult (*PFN_cuDeviceGet)(CUdevice*, int);
typedef CUresult (*PFN_cuDeviceGetAttribute)(int*, int, CUdevice);
typedef CUresult (*PFN_cuDeviceGetName)(char*, int, CUdevice);
typedef CUresult (*PFN_cuDeviceTotalMem)(size_t*, CUdevice);

GpuInfo detect_gpu()
{
    GpuInfo info;

#if defined(_WIN32) || (!defined(__APPLE__) && (defined(__linux__) || defined(__unix__)))
    // ── CUDA detection via driver API (dynamically loaded) ────────────────
#  if defined(_WIN32)
    LibHandle cuda_driver = lib_open("nvcuda.dll");
#  else
    LibHandle cuda_driver = lib_open("libcuda.so.1");
    if (!cuda_driver) cuda_driver = lib_open("libcuda.so");
#  endif

    if (cuda_driver) {
        auto pfn_cuInit           = (PFN_cuInit)              lib_sym(cuda_driver, "cuInit");
        auto pfn_cuDeviceGetCount = (PFN_cuDeviceGetCount)    lib_sym(cuda_driver, "cuDeviceGetCount");
        auto pfn_cuDeviceGet      = (PFN_cuDeviceGet)         lib_sym(cuda_driver, "cuDeviceGet");
        auto pfn_cuDeviceGetAttr  = (PFN_cuDeviceGetAttribute)lib_sym(cuda_driver, "cuDeviceGetAttribute");
        auto pfn_cuDeviceGetName  = (PFN_cuDeviceGetName)     lib_sym(cuda_driver, "cuDeviceGetName");
        auto pfn_cuDeviceTotalMem = (PFN_cuDeviceTotalMem)    lib_sym(cuda_driver, "cuDeviceTotalMem_v2");

        if (pfn_cuInit && pfn_cuDeviceGetCount && pfn_cuDeviceGet && pfn_cuDeviceGetAttr
                && pfn_cuInit(0) == CUDA_SUCCESS) {
            int device_count = 0;
            pfn_cuDeviceGetCount(&device_count);
            fprintf(stderr, "[GPU] CUDA driver found, %d device(s)\n", device_count);

            for (int i = 0; i < device_count; i++) {
                CUdevice dev;
                pfn_cuDeviceGet(&dev, i);
                int major = 0, minor = 0, sm_count = 0;
                pfn_cuDeviceGetAttr(&major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, dev);
                pfn_cuDeviceGetAttr(&minor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, dev);
                pfn_cuDeviceGetAttr(&sm_count, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, dev);
                fprintf(stderr, "[GPU] Device %d: compute %d.%d, %d SMs\n", i, major, minor, sm_count);

                if (major > 7 || (major == 7 && minor >= 5)) {
                    info.backend = GpuBackend::CUDA;
                    if (pfn_cuDeviceGetName) {
                        char name[256] = {};
                        pfn_cuDeviceGetName(name, 256, dev);
                        info.device_name = name;
                    }
                    if (pfn_cuDeviceTotalMem)
                        pfn_cuDeviceTotalMem(&info.memory_bytes, dev);
                    info.compute_capability_major = major;
                    info.compute_capability_minor = minor;
                    info.sm_count                 = sm_count;
                    info.max_chains               = std::min(sm_count * 4, 512);
                    lib_close(cuda_driver);
                    return info;
                } else {
                    fprintf(stderr, "[GPU] Compute %d.%d < 7.5 required, skipping\n",
                            major, minor);
                }
            }
        }
        lib_close(cuda_driver);
    }
#endif // Windows / Linux CUDA detection

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
