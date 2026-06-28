# env/cmake/Vcpkg.cmake
# Auto-downloads and bootstraps vcpkg, then wires it in as the toolchain.
# Must be included BEFORE the first project() call.

# Read pinned version (tag or commit hash) from env/vcpkg-version.txt
file(READ "${CMAKE_CURRENT_LIST_DIR}/../vcpkg-version.txt" VCPKG_VERSION)
string(STRIP "${VCPKG_VERSION}" VCPKG_VERSION)

set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/build/vcpkg")
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
        set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "vcpkg triplet")
    endif()
elseif(APPLE)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE _host_arch
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_host_arch STREQUAL "arm64")
            set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "vcpkg triplet")
        else()
            set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "vcpkg triplet")
        endif()
    endif()
else()
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "vcpkg triplet")
    endif()
endif()

# Overlay ports — must be set before the toolchain is loaded
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_LIST_DIR}/../ports"
    CACHE STRING "vcpkg overlay ports" FORCE)

# Wire vcpkg toolchain and installed dir into the build tree
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "vcpkg toolchain" FORCE)
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/build/vcpkg_installed"
    CACHE STRING "vcpkg install dir" FORCE)
