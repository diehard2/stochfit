cmake_minimum_required(VERSION 3.21)

# ── macOS deployment target (required for std::jthread) ──────────────────────
# Must be set before project() to correctly initialize compiler flags
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "macOS Deployment Target")
endif()

# vcpkg must be set up before project()
include(env/cmake/Vcpkg.cmake)

project(StochFit VERSION 1.0.0 LANGUAGES C CXX)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output all binaries to build/bin/Debug or build/bin/Release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# Warnings — for MSVC, /W3 is applied per-target/per-source-file below.
# add_compile_options(/W3) would leak into nvcc's CUDA FLAGS; in CUDA 13.1,
# nvcc treats bare /flag-style arguments as input file paths (since they look
# like absolute paths on Windows), triggering "single input file required".
if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# ── Release optimizations ──────────────────────────────────────────────────────
# Target: CPUs from ~2020+ (AVX2+FMA on x86, Apple Silicon on ARM)
if(MSVC)
    # /Oi: expand intrinsics; /fp:fast: aggressive FP reordering; /GL: whole-program opt
    # /arch:AVX2: AVX2 + FMA (Haswell 2013+, covers all modern chips)
    # NOTE: These flags are NOT added here via add_compile_options.
    # CMake aggregates directory/target COMPILE_OPTIONS as CUDA host flags, so any
    # flag with $<COMPILE_LANGUAGE:CXX> still leaks into nvcc's host compilation.
    # nvcc on Windows intercepts /arch as --gpu-architecture, making /arch:AVX2 fatal.
    # Instead, apply these per-target and per-source-file — see the MSVC Release
    # optimizations block below (after all targets are defined).
    add_link_options($<$<CONFIG:Release>:/LTCG>)
else()
    # -O3: full auto-vectorization; -ffast-math: aggressive FP; -flto: link-time opt
    add_compile_options(
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffast-math>
        $<$<CONFIG:Release>:-flto>
    )
    add_link_options($<$<CONFIG:Release>:-flto>)
    # Architecture tuning — x86-64-v3 = AVX2+FMA (Haswell/Zen2+, ~2019+)
    if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        add_compile_options($<$<CONFIG:Release>:-mcpu=apple-m1>)
    else()
        add_compile_options($<$<CONFIG:Release>:-march=x86-64-v3>)
    endif()
endif()

# ── Dependencies ───────────────────────────────────────────────────────────────
# CMake 4.x FindLAPACK changed cheev_ detection; use clapack CONFIG directly on Windows
if(WIN32 OR (DEFINED VCPKG_TARGET_TRIPLET))
    find_package(clapack CONFIG REQUIRED)
    # clapack exports: lapack (STATIC IMPORTED) and f2c (STATIC IMPORTED)
    # BLAS comes from openblas; use imported targets directly
    set(LAPACK_LIBRARIES lapack f2c)
else()
    find_package(LAPACK REQUIRED)
endif()

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Homebrew libomp path
    set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
endif()

find_package(OpenMP)
find_package(GTest REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
enable_testing()

# ── Optional GPU support ──────────────────────────────────────────────────────
option(STOCHFIT_USE_CUDA "Enable CUDA GPU acceleration" ON)
option(STOCHFIT_USE_METAL "Enable Metal GPU acceleration" ON)

if(STOCHFIT_USE_CUDA AND NOT APPLE)
    # Set flags BEFORE enabling CUDA so the compiler test succeeds on new MSVC.
    if(MSVC)
        set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler" CACHE STRING "CUDA flags")
    endif()

    if(NOT CMAKE_CUDA_COMPILER)
        find_program(CMAKE_CUDA_COMPILER nvcc)
    endif()

    if(CMAKE_CUDA_COMPILER)
        # CUDA 13.1 maxes out at C++20, so set it before enabling CUDA language
        set(CMAKE_CUDA_STANDARD 20)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)

        find_package(CUDAToolkit REQUIRED)
        enable_language(CUDA)
        set(STOCHFIT_HAS_CUDA TRUE)
        set(CMAKE_CUDA_RUNTIME_LIBRARY "Static")
        if(MSVC)
            # Pass /W3 wrapped via -Xcompiler so nvcc forwards it to cl.exe
            # rather than interpreting it as an input file path (CUDA 13.1 behavior)
            set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler=/W3")
        endif()
        message(STATUS "CUDA GPU acceleration enabled (CUDA ${CUDAToolkit_VERSION})")
    else()
        message(STATUS "CUDA compiler not found — GPU acceleration disabled")
    endif()
endif()

if(STOCHFIT_USE_METAL AND APPLE)
    find_library(METAL_FRAMEWORK Metal)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    if(METAL_FRAMEWORK)
        set(STOCHFIT_HAS_METAL TRUE)
        message(STATUS "Metal GPU acceleration enabled")
    else()
        message(STATUS "Metal not found — GPU acceleration disabled")
    endif()
endif()

# ── stochfit_core (static lib — CPU + runtime GPU detection) ──────────────────
# Contains all CPU code, StochFitHarness, and the GPU detection/dispatch layer.
# CUDA is probed at runtime via LoadLibrary(nvcuda.dll)/dlopen(libcuda.so)
# so this library — and stochfit.dll — have NO hard dependency on CUDA and
# load cleanly on machines with no NVIDIA GPU driver.
# The actual CUDA kernels live in stochfit_cuda_plugin.dll (see below).
add_library(stochfit_core STATIC
    src/stochfitdll/CEDP.cpp
    src/stochfitdll/ParamVector.cpp
    src/stochfitdll/ReflCalc.cpp
    src/stochfitdll/SA_Dispatcher.cpp
    src/stochfitdll/SimulatedAnnealing.cpp
    src/stochfitdll/StochFitHarness.cpp
    src/stochfitdll/gpu/gpu_detect.cpp
    src/stochfitdll/gpu/gpu_sa_runner.cpp
)
target_include_directories(stochfit_core
    PUBLIC include
    PRIVATE include/stochfit src/stochfitdll)
if(OpenMP_CXX_FOUND)
    target_link_libraries(stochfit_core PUBLIC OpenMP::OpenMP_CXX)

    # On macOS, we need to redistribute libomp.dylib.
    # FindOpenMP uses compiler flags rather than IMPORTED_LOCATION, so
    # search for the dylib directly via find_library.
    if(APPLE)
        find_library(LIBOMP_DYLIB NAMES omp libomp
            HINTS
                "${OpenMP_ROOT}/lib"
                "/opt/homebrew/opt/libomp/lib"
                "/usr/local/opt/libomp/lib"
            NO_DEFAULT_PATH)
        if(LIBOMP_DYLIB)
            message(STATUS "Will bundle libomp from: ${LIBOMP_DYLIB}")
        else()
            message(WARNING "libomp.dylib not found — cannot bundle for redistribution")
        endif()
    endif()
endif()
# ${CMAKE_DL_LIBS} is empty on Windows/macOS; provides -ldl on Linux for dlopen.
target_link_libraries(stochfit_core PUBLIC tl::expected ${CMAKE_DL_LIBS})

# ── Copy OpenMP runtime DLL on MSVC so executables run without a full VS install ─
# vcomp140.dll lives in the VS Redist directory which is not on PATH by default.
# Copying it to the output dir ensures mirefl, stochfit_tests, and stochfit.dll
# all find it at runtime (CI and end-user machines alike).
if(MSVC AND OpenMP_CXX_FOUND)
    # VCToolsRedistDir is set by vcvarsall.bat / ilammy/msvc-dev-cmd.
    # Derive it from VCToolsInstallDir as a fallback (.../Tools/MSVC/ -> .../Redist/MSVC/).
    if(NOT "$ENV{VCToolsRedistDir}" STREQUAL "")
        set(_redist "$ENV{VCToolsRedistDir}")
    elseif(NOT "$ENV{VCToolsInstallDir}" STREQUAL "")
        string(REPLACE "\\Tools\\MSVC\\" "\\Redist\\MSVC\\" _redist "$ENV{VCToolsInstallDir}")
        string(REPLACE "/Tools/MSVC/"    "/Redist/MSVC/"    _redist "${_redist}")
    endif()
    if(DEFINED _redist)
        file(TO_CMAKE_PATH "${_redist}" _redist)
        find_file(VCOMP_DLL NAMES vcomp140.dll
            PATHS
                "${_redist}/x64/Microsoft.VC143.OPENMP"
                "${_redist}/x64/Microsoft.VC142.OPENMP"
            NO_DEFAULT_PATH)
    endif()
    if(VCOMP_DLL)
        message(STATUS "Will copy OpenMP DLL: ${VCOMP_DLL}")
        add_custom_command(TARGET stochfit_core POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VCOMP_DLL}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMENT "Copying OpenMP runtime DLL (vcomp140.dll)")
    else()
        message(STATUS "OpenMP DLL not found in Redist — add VS Redist/x64/Microsoft.VC143.OPENMP to PATH if smoke test fails")
    endif()
endif()

if(STOCHFIT_HAS_METAL)
    target_sources(stochfit_core PRIVATE src/stochfitdll/gpu/stochfit_metal.mm)
    target_compile_definitions(stochfit_core PUBLIC STOCHFIT_HAS_METAL)
    target_link_libraries(stochfit_core PUBLIC ${METAL_FRAMEWORK} ${FOUNDATION_FRAMEWORK})
endif()

# ── stochfit_cuda_plugin (shared DLL — CUDA kernels only, loaded at runtime) ──
# Loaded lazily by gpu_sa_runner.cpp via LoadLibrary/dlopen after detect_gpu()
# confirms a suitable CUDA GPU is present.  Because stochfit.dll never links
# this target, CUDA static initializers cannot run at DLL-load time on
# machines without an NVIDIA driver.
if(STOCHFIT_HAS_CUDA)
    add_library(stochfit_cuda_plugin SHARED
        src/stochfitdll/gpu/stochfit_cuda.cu
    )
    target_include_directories(stochfit_cuda_plugin
        PRIVATE include include/stochfit src/stochfitdll)
    target_compile_definitions(stochfit_cuda_plugin PRIVATE STOCHFIT_HAS_CUDA)
    set_target_properties(stochfit_cuda_plugin PROPERTIES
        OUTPUT_NAME stochfit_cuda_plugin
        CUDA_ARCHITECTURES "75;86;89;100")
    target_link_libraries(stochfit_cuda_plugin PRIVATE CUDA::cudart tl::expected)
endif()

# ── stochfit_dll (shared lib — FFI exports only, loaded by Electron) ──────────
add_library(stochfit_dll SHARED
    src/stochfitdll/StochFitDll.cpp
)
target_include_directories(stochfit_dll
    PRIVATE include/stochfit src/stochfitdll)
set_target_properties(stochfit_dll PROPERTIES
    OUTPUT_NAME stochfit
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(stochfit_dll PRIVATE stochfit_core)

# Bundle libomp.dylib alongside the DLL so Electron can load it on macOS
if(APPLE AND LIBOMP_DYLIB)
    add_custom_command(TARGET stochfit_dll POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBOMP_DYLIB}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libomp.dylib"
        COMMENT "Copying libomp.dylib to output directory"
    )
endif()

# ── levmar (from vcpkg overlay port) ─────────────────────────────────────────
# The port doesn't generate a CMake config file, so we create an imported
# target manually from the vcpkg-installed artifacts.
find_path(LEVMAR_INCLUDE_DIR levmar.h PATH_SUFFIXES levmar REQUIRED)
find_library(LEVMAR_LIBRARY_RELEASE NAMES levmar
    PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
    NO_DEFAULT_PATH REQUIRED)
find_library(LEVMAR_LIBRARY_DEBUG NAMES levmar
    PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib"
    NO_DEFAULT_PATH)

add_library(levmar STATIC IMPORTED GLOBAL)
set_target_properties(levmar PROPERTIES
    IMPORTED_LOCATION           "${LEVMAR_LIBRARY_RELEASE}"
    IMPORTED_LOCATION_RELEASE   "${LEVMAR_LIBRARY_RELEASE}"
    INTERFACE_INCLUDE_DIRECTORIES "${LEVMAR_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES    "${LAPACK_LIBRARIES}"
)
if(LEVMAR_LIBRARY_DEBUG)
    set_target_properties(levmar PROPERTIES IMPORTED_LOCATION_DEBUG "${LEVMAR_LIBRARY_DEBUG}")
endif()

# ── levmardll (shared lib) ────────────────────────────────────────────────────
add_library(levmardll SHARED
    src/levmardll/FastReflCalc.cpp
    src/levmardll/LevMardll.cpp
    src/levmardll/ReflCalc.cpp
    src/levmardll/RhoCalc.cpp
)
target_include_directories(levmardll
    PUBLIC include
    PRIVATE include/levmar)
set_target_properties(levmardll PROPERTIES
    OUTPUT_NAME levmardll
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(levmardll PUBLIC stochfit_core levmar)

# ── mirefl (console executable) ───────────────────────────────────────────────
add_executable(mirefl tests/MIRefl.cpp)
target_include_directories(mirefl PRIVATE include/stochfit src/stochfitdll)
target_link_libraries(mirefl PRIVATE stochfit_core)

add_executable(stochfit_tests tests/test_reflectivity.cpp)
target_link_libraries(stochfit_tests PRIVATE stochfit_core GTest::gtest GTest::gtest_main)
target_compile_features(stochfit_tests PRIVATE cxx_std_20)
add_test(NAME stochfit_tests COMMAND stochfit_tests)

# ── MSVC Release optimizations ────────────────────────────────────────────────
# stochfit_core has no CUDA sources so target_compile_options is safe on it.
# stochfit_cuda_plugin contains only the .cu file; /arch:AVX2 etc. must be
# applied per-source-file via set_source_files_properties because CMake
# aggregates target COMPILE_OPTIONS into nvcc host flags on Windows.
if(MSVC)
    set(_msvc_opts
        /W3
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/fp:fast>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/arch:AVX2>
    )
    target_compile_options(stochfit_core  PRIVATE ${_msvc_opts})
    target_compile_options(stochfit_dll   PRIVATE ${_msvc_opts})
    target_compile_options(levmardll      PRIVATE ${_msvc_opts})
    target_compile_options(mirefl         PRIVATE ${_msvc_opts})
    target_compile_options(stochfit_tests PRIVATE ${_msvc_opts})
    # stochfit_cuda_plugin (.cu only) intentionally omitted: nvcc has its own
    # Release optimizations and interprets MSVC-style /flags as input file paths.
endif()

# ── compile_commands.json ─────────────────────────────────────────────────────
# Copy to build/ so clangd always finds it regardless of active config (Debug/Release)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_BINARY_DIR}/../compile_commands.json"
        COMMENT "Copying compile_commands.json to build/"
    )
endif()

# ── Electron GUI ──────────────────────────────────────────────────────────────
# cmake --build build            → C++ only (fast, default)
# cmake --build build --target gui → npm install + forge make → build/electron/
find_program(NPM npm.cmd npm
    HINTS
        "C:/Program Files/nodejs"
        "C:/Program Files (x86)/nodejs"
        "/usr/local/bin"
        "/opt/homebrew/bin"
    NO_CACHE)
if(NPM)
    # Tracked install step: only re-runs when package.json / package-lock.json change
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/gui/node_modules/.package-lock.json
        COMMAND ${NPM} install
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gui
        DEPENDS
            ${CMAKE_SOURCE_DIR}/gui/package.json
            ${CMAKE_SOURCE_DIR}/gui/package-lock.json
        COMMENT "Installing npm dependencies"
        VERBATIM
    )

    add_custom_target(gui
        COMMAND ${NPM} run make
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gui
        DEPENDS
            ${CMAKE_SOURCE_DIR}/gui/node_modules/.package-lock.json
            stochfit_dll
            levmardll
        COMMENT "Packaging Electron GUI → build/electron/"
        VERBATIM
    )
else()
    message(STATUS "npm not found — 'gui' target unavailable")
endif()
