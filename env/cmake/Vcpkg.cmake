# env/cmake/Vcpkg.cmake
# Auto-downloads and bootstraps vcpkg, then wires it in as the toolchain.
# Must be included BEFORE the first project() call.

# Read pinned version (tag or commit hash) from env/vcpkg-version.txt
file(READ "${CMAKE_CURRENT_LIST_DIR}/../vcpkg-version.txt" VCPKG_VERSION)
string(STRIP "${VCPKG_VERSION}" VCPKG_VERSION)

set(VCPKG_DIR "${CMAKE_BINARY_DIR}/vcpkg")
# Stamp lives inside VCPKG_DIR so it is included when build/vcpkg is cached
set(VCPKG_STAMP "${VCPKG_DIR}/.bootstrap-stamp")

# Only clone + bootstrap when the stamp is absent or records a different version
set(_VCPKG_CACHED_VERSION "")
if(EXISTS "${VCPKG_STAMP}")
    file(READ "${VCPKG_STAMP}" _VCPKG_CACHED_VERSION)
    string(STRIP "${_VCPKG_CACHED_VERSION}" _VCPKG_CACHED_VERSION)
endif()
if(NOT _VCPKG_CACHED_VERSION STREQUAL VCPKG_VERSION)
    message(STATUS "[vcpkg] Cloning vcpkg @ ${VCPKG_VERSION} into ${VCPKG_DIR}")
    find_package(Git REQUIRED)
    if(EXISTS "${VCPKG_DIR}/.git")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} fetch --tags
            WORKING_DIRECTORY "${VCPKG_DIR}"
        )
    else()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone
                --filter=blob:none
                --no-checkout
                https://github.com/microsoft/vcpkg.git
                "${VCPKG_DIR}"
        )
    endif()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} checkout "${VCPKG_VERSION}"
        WORKING_DIRECTORY "${VCPKG_DIR}"
    )

    if(WIN32)
        set(VCPKG_BOOTSTRAP "${VCPKG_DIR}/bootstrap-vcpkg.bat")
    else()
        set(VCPKG_BOOTSTRAP "${VCPKG_DIR}/bootstrap-vcpkg.sh")
    endif()

    execute_process(
        COMMAND "${VCPKG_BOOTSTRAP}" -disableMetrics
        WORKING_DIRECTORY "${VCPKG_DIR}"
        RESULT_VARIABLE VCPKG_BOOTSTRAP_RESULT
    )
    if(NOT VCPKG_BOOTSTRAP_RESULT EQUAL 0)
        message(FATAL_ERROR "[vcpkg] Bootstrap failed (exit code ${VCPKG_BOOTSTRAP_RESULT}). "
            "Install curl, zip, unzip, tar and a C++ compiler, then re-run cmake.")
    endif()

    # Write stamp only after a successful bootstrap
    file(WRITE "${VCPKG_STAMP}" "${VCPKG_VERSION}\n")
endif()

# Select triplet per platform (dynamic CRT on Windows for better compatibility)
if(WIN32)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet")
    endif()
elseif(APPLE)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "vcpkg triplet")
    endif()
else()
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "vcpkg triplet")
    endif()
endif()

# Wire vcpkg toolchain and installed dir into the build tree
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "vcpkg toolchain" FORCE)
set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed"
    CACHE STRING "vcpkg install dir" FORCE)

# ── Optional GitHub Packages binary cache ─────────────────────────────────────
# Activates automatically when a GitHub token is available in the environment.
# Owner is derived from the git remote; no extra configuration needed.
#
# Token priority: VCPKG_GITHUB_PACKAGES_TOKEN → GITHUB_TOKEN

# Resolve token (explicit override, then the standard GITHUB_TOKEN)
foreach(_tok VCPKG_GITHUB_PACKAGES_TOKEN GITHUB_TOKEN)
    if(NOT "$ENV{${_tok}}" STREQUAL "")
        set(_VCPKG_GH_TOKEN "$ENV{${_tok}}")
        break()
    endif()
endforeach()

# Derive owner from git remote URL (supports https:// and git@)
if(NOT _VCPKG_GH_TOKEN STREQUAL "")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} remote get-url origin
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE _git_remote_url
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        # Match both https://github.com/OWNER/... and git@github.com:OWNER/...
        if(_git_remote_url MATCHES "github\\.com[:/]([^/]+)/")
            set(_VCPKG_GH_OWNER "${CMAKE_MATCH_1}")
        endif()
    endif()
endif()

if(NOT _VCPKG_GH_TOKEN STREQUAL "" AND NOT _VCPKG_GH_OWNER STREQUAL "")
    set(_VCPKG_NUGET_FEED
        "https://nuget.pkg.github.com/${_VCPKG_GH_OWNER}/index.json")
    message(STATUS "[vcpkg] Binary cache → ${_VCPKG_NUGET_FEED}")

    find_program(DOTNET_EXE dotnet)
    if(NOT DOTNET_EXE)
        message(STATUS "[vcpkg] dotnet not found — binary cache skipped. Install dotnet-sdk-8.0 to enable.")
        return()
    endif()

    # Register feed; if name already exists dotnet exits non-zero → update credentials
    execute_process(
        COMMAND ${DOTNET_EXE} nuget add source "${_VCPKG_NUGET_FEED}"
            --name GitHubPackages
            --username "${_VCPKG_GH_OWNER}"
            --password "${_VCPKG_GH_TOKEN}"
            --store-password-in-clear-text
        RESULT_VARIABLE _nuget_result
        OUTPUT_QUIET ERROR_QUIET
    )
    if(NOT _nuget_result EQUAL 0)
        execute_process(
            COMMAND ${DOTNET_EXE} nuget update source GitHubPackages
                --username "${_VCPKG_GH_OWNER}"
                --password "${_VCPKG_GH_TOKEN}"
                --store-password-in-clear-text
            OUTPUT_QUIET ERROR_QUIET
        )
    endif()

    set(VCPKG_BINARY_SOURCES "clear;nuget,GitHubPackages,readwrite"
        CACHE STRING "vcpkg binary cache" FORCE)
endif()
