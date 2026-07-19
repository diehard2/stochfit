# Ensures cl.exe/rc.exe/mt.exe are resolvable, running vcvarsall.bat if needed.
# Use as VCPKG_CHAINLOAD_TOOLCHAIN_FILE on Windows.
#
# If a VS dev environment is already active (VS Code CMake Tools kit,
# ilammy/msvc-dev-cmd in CI, a Developer Prompt, ...), cl.exe/rc.exe/mt.exe
# are already correctly on PATH and CMake's own detection will find them —
# we leave CMAKE_C_COMPILER/CMAKE_RC_COMPILER/CMAKE_MT untouched so we can't
# override a correct auto-detected path with a wrong hand-built one. We only
# step in and run vcvarsall.bat ourselves when nothing is active, which is
# the case for a bare `cmake --preset windows` on a plain CI runner.

if(NOT WIN32)
  return()
endif()

find_program(_cl_on_path cl.exe)
if(_cl_on_path)
  message(STATUS "MSVC environment already active (cl.exe found on PATH: ${_cl_on_path})")
  return()
endif()

# Use vswhere to locate vcvarsall.bat (works for any VS version/edition)
# Note: ProgramFiles(x86) cannot be used directly in $ENV{} due to parentheses
set(_vs_installer "C:/Program Files (x86)/Microsoft Visual Studio/Installer")
find_program(VSWHERE vswhere.exe PATHS "${_vs_installer}" NO_DEFAULT_PATH)

if(VSWHERE)
  execute_process(
    COMMAND "${VSWHERE}" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
            -find VC/Auxiliary/Build/vcvarsall.bat
    OUTPUT_VARIABLE VCVARSALL
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  string(REPLACE "\\" "/" VCVARSALL "${VCVARSALL}")
endif()

# Fallback to hardcoded paths if vswhere didn't find it
if(NOT VCVARSALL OR NOT EXISTS "${VCVARSALL}")
  find_program(VCVARSALL vcvarsall.bat PATHS
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Community/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Professional/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Enterprise/VC/Auxiliary/Build"
    NO_DEFAULT_PATH
  )
endif()

if(NOT VCVARSALL OR NOT EXISTS "${VCVARSALL}")
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
  "echo SDKVERBIN=%WindowsSdkVerBinPath%\r\n"
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

extract_vcvar(VCTOOLS   _vc_tools)
extract_vcvar(SDKBIN    _sdk_bin)
extract_vcvar(SDKVERBIN _sdk_verbin)
extract_vcvar(VCLIB     _vc_lib)
extract_vcvar(VCINCLUDE _vc_include)
extract_vcvar(VCPATH    _vc_path)

# WindowsSdkBinPath is often the unversioned "Windows Kits/10/bin/" directory,
# which has no rc.exe/mt.exe directly under bin/x64 on many SDK installs — those
# live under the versioned bin/<version>/x64 subfolder. Prefer that when set,
# and verify the result actually contains rc.exe, falling back to a glob for
# the newest installed SDK version if not (some vcvarsall variants don't
# export WindowsSdkVerBinPath at all).
if(_sdk_verbin)
  set(_sdk_bin "${_sdk_verbin}")
endif()
if(NOT EXISTS "${_sdk_bin}/x64/rc.exe")
  file(GLOB _sdk_versions LIST_DIRECTORIES true
       "C:/Program Files (x86)/Windows Kits/10/bin/10.*")
  set(_sdk_bin "")
  foreach(_v ${_sdk_versions})
    if(EXISTS "${_v}/x64/rc.exe" AND _v STRGREATER _sdk_bin)
      set(_sdk_bin "${_v}")
    endif()
  endforeach()
endif()
if(NOT _sdk_bin)
  message(FATAL_ERROR "Could not find a Windows SDK bin directory containing rc.exe/mt.exe.")
endif()

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
