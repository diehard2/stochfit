cmake_minimum_required(VERSION 3.21)

# vcpkg must be set up before project()
include(cmake/Vcpkg.cmake)

project(StochFit VERSION 1.0.0 LANGUAGES C CXX)

# ── Global settings ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# ── Dependencies ───────────────────────────────────────────────────────────────
find_package(LAPACK REQUIRED)
find_package(OpenMP)
find_package(GTest REQUIRED)
enable_testing()

# ── stochfit_core (static lib — StochFitdll sources) ──────────────────────────
add_library(stochfit_core STATIC
    src/stochfitdll/CEDP.cpp
    src/stochfitdll/ParamVector.cpp
    src/stochfitdll/ReflCalc.cpp
    src/stochfitdll/SA_Dispatcher.cpp
    src/stochfitdll/SimulatedAnnealing.cpp
    src/stochfitdll/StochFitDll.cpp
    src/stochfitdll/StochFitHarness.cpp
)
target_include_directories(stochfit_core
    PUBLIC include
    PRIVATE include/stochfit/stochfitdll)
set_target_properties(stochfit_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
if(OpenMP_CXX_FOUND)
    target_link_libraries(stochfit_core PUBLIC OpenMP::OpenMP_CXX)
endif()

# ── levmar (static lib — pure C Lourakis library) ─────────────────────────────
# Axb_core.c, lm_core.c, etc. are #include'd by the non-_core files — do NOT
# add them as compilation units.
add_library(levmar STATIC
    src/levmar/Axb.c
    src/levmar/lm.c
    src/levmar/lmbc.c
    src/levmar/lmblec.c
    src/levmar/lmlec.c
    src/levmar/misc.c
)
target_compile_definitions(levmar PRIVATE HAVE_LAPACK)
target_include_directories(levmar
    PUBLIC include
    PRIVATE include/stochfit/levmar)
target_link_libraries(levmar PRIVATE ${LAPACK_LIBRARIES})

# ── levmardll (static lib) ────────────────────────────────────────────────────
add_library(levmardll STATIC
    src/levmardll/FastReflCalc.cpp
    src/levmardll/LMRoutines.cpp
    src/levmardll/LevMardll.cpp
    src/levmardll/ReflCalc.cpp
    src/levmardll/RhoCalc.cpp
)
target_include_directories(levmardll
    PUBLIC include
    PRIVATE include/stochfit/levmardll include/stochfit/levmar)
set_target_properties(levmardll PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(levmardll PUBLIC stochfit_core levmar)

# ── mirefl (console executable) ───────────────────────────────────────────────
add_executable(mirefl
    src/mirefl/MIRefl.cpp
)
# CEDP and CReflCalc come from stochfit_core (which transitively brings OpenMP).
# src/mirefl is on the include path for MDParamVec.h / targetver.h.
# include/stochfit/stochfitdll resolves the bare "CEDP.h" / "ReflCalc.h" includes.
target_include_directories(mirefl PRIVATE src/mirefl include include/stochfit/stochfitdll)
target_link_libraries(mirefl PRIVATE stochfit_core)

# ── Shared library shims ───────────────────────────────────────────────────────
add_library(stochfit_shared SHARED src/shim/stochfit_shim.cpp)
set_target_properties(stochfit_shared PROPERTIES
    OUTPUT_NAME stochfit
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(stochfit_shared PRIVATE include)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(stochfit_shared PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:stochfit_core> -Wl,--no-whole-archive)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(stochfit_shared PRIVATE
        -Wl,-force_load,$<TARGET_FILE:stochfit_core>)
elseif(MSVC)
    target_link_options(stochfit_shared PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:stochfit_core>)
endif()
target_link_libraries(stochfit_shared PRIVATE stochfit_core)

add_executable(stochfit_tests
    tests/test_reflectivity.cpp)
target_include_directories(stochfit_tests PRIVATE include include/stochfit/stochfitdll)
target_link_libraries(stochfit_tests PRIVATE stochfit_core GTest::gtest GTest::gtest_main)
target_compile_features(stochfit_tests PRIVATE cxx_std_23)
add_test(NAME stochfit_tests COMMAND stochfit_tests)

add_library(levmardll_shared SHARED src/shim/levmardll_shim.cpp)
set_target_properties(levmardll_shared PROPERTIES
    OUTPUT_NAME levmardll
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(levmardll_shared PRIVATE include)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(levmardll_shared PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:levmardll> -Wl,--no-whole-archive)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(levmardll_shared PRIVATE
        -Wl,-force_load,$<TARGET_FILE:levmardll>)
elseif(MSVC)
    target_link_options(levmardll_shared PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:levmardll>)
endif()
target_link_libraries(levmardll_shared PRIVATE levmardll)

# ── Electron GUI ──────────────────────────────────────────────────────────────
# cmake --build build            → C++ only (fast, default)
# cmake --build build --target gui → npm install + forge make → build/electron/
find_program(NPM npm)
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
            stochfit_shared
            levmardll_shared
        COMMENT "Packaging Electron GUI → build/electron/"
        VERBATIM
    )
else()
    message(STATUS "npm not found — 'gui' target unavailable")
endif()
