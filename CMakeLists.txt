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


# Warnings — per-target via target_compile_options to avoid leaking into any future tool flags.
if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# ── Release optimizations ──────────────────────────────────────────────────────
# Target: CPUs from ~2020+ (AVX2+FMA on x86, Apple Silicon on ARM)
if(MSVC)
    # /Oi: expand intrinsics; /fp:fast: aggressive FP reordering
    # /arch:AVX2: AVX2 + FMA (Haswell 2013+, covers all modern chips)
    # Applied per-target below (not via add_compile_options) to avoid leaking into future tools.
    # /LTCG disabled: hurts OpenMP throughput (~8% regression measured vs without it)
    # add_link_options($<$<CONFIG:Release>:/LTCG>)
else()
    # -O3: full auto-vectorization; -ffast-math: aggressive FP; -flto: link-time opt
    add_compile_options(
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffast-math>
        $<$<CONFIG:Release>:-flto=auto>
    )
    add_link_options($<$<CONFIG:Release>:-flto=auto>)
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
find_package(flatbuffers CONFIG REQUIRED)
enable_testing()

# ── FlatBuffers code generation ────────────────────────────────────────────────
# flatbuffers CONFIG sets FLATBUFFERS_FLATC_EXECUTABLE; fall back to find_program
# if the variable is empty (vcpkg installs flatc under the host triplet's tools/).
if(NOT FLATBUFFERS_FLATC_EXECUTABLE)
    find_program(FLATBUFFERS_FLATC_EXECUTABLE flatc
        HINTS
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/flatbuffers"
            "${VCPKG_INSTALLED_DIR}/x64-windows/tools/flatbuffers"
            "${VCPKG_INSTALLED_DIR}/x64-windows-static-md/tools/flatbuffers"
            "${VCPKG_INSTALLED_DIR}/x64-osx/tools/flatbuffers"
            "${VCPKG_INSTALLED_DIR}/arm64-osx/tools/flatbuffers"
            "${VCPKG_INSTALLED_DIR}/x64-linux/tools/flatbuffers"
    )
endif()
if(NOT FLATBUFFERS_FLATC_EXECUTABLE)
    message(FATAL_ERROR "flatc compiler not found. Run 'cmake --preset <preset>' to let vcpkg install flatbuffers first.")
endif()
set(FLATC "${FLATBUFFERS_FLATC_EXECUTABLE}")

set(FBS_SCHEMA  "${CMAKE_SOURCE_DIR}/schema/stochfit.fbs")
set(FBS_CPP_OUT "${CMAKE_SOURCE_DIR}/schema/generated")
set(FBS_TS_OUT  "${CMAKE_SOURCE_DIR}/gui/src/main/native/generated")

file(MAKE_DIRECTORY "${FBS_CPP_OUT}")
file(MAKE_DIRECTORY "${FBS_TS_OUT}")

add_custom_command(
    OUTPUT  "${FBS_CPP_OUT}/stochfit_generated.h"
            "${FBS_TS_OUT}/stoch-fit-proto/init-request.ts"
    COMMAND "${FLATC}" --cpp -o "${FBS_CPP_OUT}" "${FBS_SCHEMA}"
    COMMAND "${FLATC}" --ts  -o "${FBS_TS_OUT}" "${FBS_SCHEMA}"
    DEPENDS "${FBS_SCHEMA}"
    COMMENT "Generating FlatBuffers code from schema/stochfit.fbs"
    VERBATIM
)
add_custom_target(flatbuffers_gen
    DEPENDS "${FBS_CPP_OUT}/stochfit_generated.h"
            "${FBS_TS_OUT}/stoch-fit-proto/init-request.ts"
)

# ── stochfit_core (static lib) ────────────────────────────────────────────────
add_library(stochfit_core STATIC
    src/stochfitdll/AnnealPolicies.cpp
    src/stochfitdll/CEDP.cpp
    src/stochfitdll/ParameterStepper.cpp
    src/stochfitdll/ParamVector.cpp
    src/stochfitdll/ReflectivityObjective.cpp
    src/stochfitdll/StochFitHarness.cpp
    src/stochfitdll/UnifiedReflectivity.cpp
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

# ── stochfit_dll (shared lib — FFI exports only, loaded by Electron) ──────────
add_library(stochfit_dll SHARED
    src/stochfitdll/StochFitDll.cpp
    src/stochfitdll/SettingsStruct.cpp
)
add_dependencies(stochfit_dll flatbuffers_gen)
target_include_directories(stochfit_dll
    PRIVATE include/stochfit src/stochfitdll "${CMAKE_SOURCE_DIR}/schema")
set_target_properties(stochfit_dll PROPERTIES
    OUTPUT_NAME stochfit
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(stochfit_dll PRIVATE stochfit_core flatbuffers::flatbuffers)

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
    src/levmardll/BoxLayerBuild.cpp
    src/levmardll/LevMardll.cpp
    src/levmardll/RhoCalc.cpp
    src/levmardll/SettingsBridge.cpp
)
add_dependencies(levmardll flatbuffers_gen)
target_include_directories(levmardll
    PUBLIC include
    PRIVATE include/levmar "${CMAKE_SOURCE_DIR}/schema")
set_target_properties(levmardll PROPERTIES
    OUTPUT_NAME levmardll
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(levmardll PUBLIC stochfit_core levmar flatbuffers::flatbuffers)

# ── mirefl (console executable) ───────────────────────────────────────────────
add_executable(mirefl tests/MIRefl.cpp)
target_include_directories(mirefl PRIVATE include/stochfit src/stochfitdll)
target_link_libraries(mirefl PRIVATE stochfit_core)

add_executable(stochfit_tests tests/test_reflectivity.cpp)
target_link_libraries(stochfit_tests PRIVATE stochfit_core GTest::gtest GTest::gtest_main)
target_compile_features(stochfit_tests PRIVATE cxx_std_20)
add_test(NAME stochfit_tests COMMAND stochfit_tests)

add_executable(stochfit_profile tests/stochfit_profile.cpp)
target_include_directories(stochfit_profile PRIVATE include/stochfit src/stochfitdll)
target_link_libraries(stochfit_profile PRIVATE stochfit_core)
target_compile_definitions(stochfit_profile PRIVATE
    RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources")

# ── MSVC Release optimizations ────────────────────────────────────────────────
if(MSVC)
    # CMake's default CMAKE_CXX_FLAGS_RELEASE includes /Ob2; replace with /Ob3
    # to avoid D9025 "overriding /Ob2 with /Ob3" warning.
    foreach(_lang CXX C)
        string(REPLACE "/Ob2" "/Ob3" CMAKE_${_lang}_FLAGS_RELEASE "${CMAKE_${_lang}_FLAGS_RELEASE}")
    endforeach()

    set(_msvc_opts
        /W3
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/fp:fast>
        $<$<CONFIG:Release>:/arch:AVX2>
        $<$<CONFIG:RelWithDebInfo>:/Oi>
        $<$<CONFIG:RelWithDebInfo>:/fp:fast>
        $<$<CONFIG:RelWithDebInfo>:/arch:AVX2>
        $<$<CONFIG:RelWithDebInfo>:/Oy->
    )
    target_compile_options(stochfit_core    PRIVATE ${_msvc_opts})
    target_compile_options(stochfit_dll     PRIVATE ${_msvc_opts})
    target_compile_options(levmardll        PRIVATE ${_msvc_opts})
    target_compile_options(mirefl           PRIVATE ${_msvc_opts})
    target_compile_options(stochfit_tests   PRIVATE ${_msvc_opts})
    target_compile_options(stochfit_profile PRIVATE ${_msvc_opts})
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
