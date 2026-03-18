#include "gpu_sa_runner.h"
#include <cstdio>
#include <string>

// Platform-specific dynamic loading for the CUDA plugin DLL.
// stochfit_cuda_plugin.{dll,so} is loaded lazily the first time a CUDA
// backend is requested.  stochfit_core never links CUDA directly, so
// stochfit.dll loads cleanly on machines with no NVIDIA GPU driver.
#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
using LibHandle = HMODULE;
static LibHandle lib_open(const char* n)    { return LoadLibraryA(n); }
static void*     lib_sym(LibHandle h, const char* n) { return (void*)GetProcAddress(h, n); }
#elif !defined(__APPLE__)
#  include <dlfcn.h>
using LibHandle = void*;
static LibHandle lib_open(const char* n)    { return dlopen(n, RTLD_LAZY | RTLD_LOCAL); }
static void*     lib_sym(LibHandle h, const char* n) { return dlsym(h, n); }
#endif

#if defined(STOCHFIT_HAS_METAL)
#  include "stochfit_metal.h"
#endif

typedef GpuSARunner* (*PFN_stochfit_cuda_create_runner)();

// Returns the directory containing this DLL/shared-object so we can find
// stochfit_cuda_plugin.{dll,so} sitting alongside it.
static std::string get_this_module_dir()
{
#if defined(_WIN32)
    HMODULE hmod = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&get_this_module_dir, &hmod);
    char buf[MAX_PATH] = {};
    GetModuleFileNameA(hmod, buf, MAX_PATH);
    char* sep = strrchr(buf, '\\');
    if (sep) sep[1] = '\0';
    return buf;
#elif !defined(__APPLE__)
    Dl_info di = {};
    dladdr((const void*)&get_this_module_dir, &di);
    std::string p = di.dli_fname ? di.dli_fname : "";
    auto pos = p.rfind('/');
    return (pos != std::string::npos) ? p.substr(0, pos + 1) : "./";
#else
    return "";  // macOS: CUDA plugin not used
#endif
}

// Load the CUDA plugin once; return its factory fn or nullptr on failure.
// The library handle is kept alive for the process lifetime (static local).
static PFN_stochfit_cuda_create_runner load_cuda_plugin()
{
#if defined(__APPLE__)
    return nullptr;  // No CUDA on Apple; Metal is used directly
#else
    std::string dir  = get_this_module_dir();
#  if defined(_WIN32)
    std::string path = dir + "stochfit_cuda_plugin.dll";
#  else
    std::string path = dir + "libstochfit_cuda_plugin.so";
#  endif
    // C++11 static-local init is thread-safe and runs exactly once.
    static LibHandle h = lib_open(path.c_str());
    if (!h) {
        fprintf(stderr, "[GPU] CUDA plugin not found: %s\n", path.c_str());
        return nullptr;
    }
    auto fn = (PFN_stochfit_cuda_create_runner)lib_sym(h, "stochfit_cuda_create_runner");
    if (!fn)
        fprintf(stderr, "[GPU] stochfit_cuda_create_runner not found in plugin\n");
    return fn;
#endif
}

std::unique_ptr<GpuSARunner> GpuSARunner::create(GpuBackend backend)
{
    if (backend == GpuBackend::CUDA) {
        auto fn = load_cuda_plugin();
        if (fn) return std::unique_ptr<GpuSARunner>(fn());
        return nullptr;
    }

#if defined(STOCHFIT_HAS_METAL)
    if (backend == GpuBackend::Metal)
        return create_metal_runner();
#endif

    return nullptr;
}
