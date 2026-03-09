# Sets up the MSVC environment by running vcvarsall.bat if not already active.
# Use as VCPKG_CHAINLOAD_TOOLCHAIN_FILE on Windows.

if(NOT WIN32)
  return()
endif()

# Search for vcvarsall.bat across common VS installation paths
find_program(VCVARSALL vcvarsall.bat PATHS
  "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Community/VC/Auxiliary/Build"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Professional/VC/Auxiliary/Build"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Enterprise/VC/Auxiliary/Build"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/17/Community/VC/Auxiliary/Build"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/17/Professional/VC/Auxiliary/Build"
  "$ENV{ProgramFiles}/Microsoft Visual Studio/17/Enterprise/VC/Auxiliary/Build"
  NO_DEFAULT_PATH
)

if(NOT VCVARSALL)
  message(FATAL_ERROR "Could not find vcvarsall.bat. Install Visual Studio with the C++ workload.")
endif()

message(STATUS "Running vcvarsall.bat: ${VCVARSALL}")

# Write a helper batch that silences vcvarsall output and echoes only what we need
set(_vcvars_bat "${CMAKE_BINARY_DIR}/vcvars_query.bat")
file(WRITE "${_vcvars_bat}"
  "@echo off\r\n"
  "call \"${VCVARSALL}\" x64 >nul 2>&1\r\n"
  "echo VCTOOLS=%VCToolsInstallDir%\r\n"
  "echo SDKBIN=%WindowsSdkBinPath%\r\n"
  "echo VCLIB=%LIB%\r\n"
  "echo VCINCLUDE=%INCLUDE%\r\n"
  "echo VCPATH=%PATH%\r\n"
)

execute_process(
  COMMAND cmd /c "${_vcvars_bat}"
  OUTPUT_VARIABLE vcvars_out
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Parse the labelled output
macro(extract_vcvar label outvar)
  string(REGEX MATCH "${label}=([^\r\n]*)" _ "${vcvars_out}")
  string(STRIP "${CMAKE_MATCH_1}" ${outvar})
  string(REPLACE "\\" "/" ${outvar} "${${outvar}}")
  string(REGEX REPLACE "[/]+$" "" ${outvar} "${${outvar}}")
endmacro()

extract_vcvar(VCTOOLS  _vc_tools)
extract_vcvar(SDKBIN   _sdk_bin)
extract_vcvar(VCLIB    _vc_lib)
extract_vcvar(VCINCLUDE _vc_include)
extract_vcvar(VCPATH   _vc_path)

# Apply LIB, INCLUDE, and PATH to the CMake process environment
set(ENV{LIB}     "${_vc_lib}")
set(ENV{INCLUDE} "${_vc_include}")
set(ENV{PATH}    "${_vc_path}")

# Also inject include and lib paths explicitly into CMake so Ninja picks them up
# (ENV vars are not propagated to Ninja subprocesses at build time)
include_directories(SYSTEM ${_vc_include})
link_directories(${_vc_lib})

# Set compiler tools as CMake cache entries
set(CMAKE_C_COMPILER   "${_vc_tools}/bin/HostX64/x64/cl.exe" CACHE FILEPATH "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER "${_vc_tools}/bin/HostX64/x64/cl.exe" CACHE FILEPATH "C++ compiler" FORCE)
set(CMAKE_RC_COMPILER  "${_sdk_bin}/x64/rc.exe"              CACHE FILEPATH "RC compiler"  FORCE)
set(CMAKE_MT           "${_sdk_bin}/x64/mt.exe"              CACHE FILEPATH "Manifest tool" FORCE)

message(STATUS "MSVC environment ready")
message(STATUS "  cl: ${_vc_tools}/bin/HostX64/x64/cl.exe")
message(STATUS "  rc: ${_sdk_bin}/x64/rc.exe")
