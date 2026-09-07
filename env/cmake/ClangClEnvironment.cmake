# Ensures clang-cl.exe/rc.exe/mt.exe are resolvable, running vcvarsall.bat if
# needed, then points CMAKE_C/CXX_COMPILER at clang-cl instead of cl.exe.
# Use as VCPKG_CHAINLOAD_TOOLCHAIN_FILE on the windows-clang preset.
#
# This is an A/B experiment against the "windows" (cl.exe/vcomp) preset: does
# switching to clang-cl (a) dodge an MSVC-14.51-specific internal compiler
# error hit by an unrelated dependency probe, and (b) get libomp (real
# OpenMP 4.x, OMP_PLACES/OMP_PROC_BIND-capable) instead of vcomp, matching
# what Linux/macOS already have? Still needs cl.exe's own INCLUDE/LIB/rc.exe/
# mt.exe from vcvarsall.bat -- clang-cl targets the MSVC ABI and consumes the
# same Windows SDK/UCRT headers and libs, it just replaces the compiler
# frontend itself.

if(NOT WIN32)
  return()
endif()

# Point OpenMP at LLVM's own libomp rather than the MSVC libomp.lib that would
# otherwise be found first on LIB. That MSVC import library resolves to
# libomp140.x86_64.dll, which exists only in System32 (put there by the VC++
# Redistributable) or under VC/Redist/.../debug_nonredist -- and debug_nonredist
# is, as the name says, NOT redistributable. Neither is deployable app-local.
# LLVM's own libomp.dll is Apache-2.0 WITH LLVM-exception and ships fine next to
# the executable, which is what the Electron installer needs.
macro(stochfit_use_llvm_openmp _clangcl)
  get_filename_component(_llvm_bin  "${_clangcl}" DIRECTORY)   # .../Llvm/x64/bin
  get_filename_component(_llvm_root "${_llvm_bin}" DIRECTORY)  # .../Llvm/x64
  if(EXISTS "${_llvm_root}/lib/libomp.lib")
    set(OpenMP_libomp_LIBRARY "${_llvm_root}/lib/libomp.lib"
        CACHE FILEPATH "LLVM OpenMP import library" FORCE)
  endif()
  # Consumed by CMakeLists.txt to copy libomp.dll into the output dir.
  set(STOCHFIT_LLVM_BIN_DIR "${_llvm_bin}"
      CACHE PATH "LLVM toolchain bin dir (source of libomp.dll)" FORCE)
endmacro()

# NO_CACHE: see the identical rationale in MsvcEnvironment.cmake -- must
# re-check every configure, not just the first one.
find_program(_clangcl_on_path clang-cl.exe NO_CACHE)
if(_clangcl_on_path AND NOT "$ENV{INCLUDE}" STREQUAL "")
  message(STATUS "clang-cl environment already active (found on PATH: ${_clangcl_on_path})")
  set(CMAKE_C_COMPILER   "${_clangcl_on_path}" CACHE FILEPATH "C compiler"   FORCE)
  set(CMAKE_CXX_COMPILER "${_clangcl_on_path}" CACHE FILEPATH "C++ compiler" FORCE)
  stochfit_use_llvm_openmp("${_clangcl_on_path}")
  return()
endif()

# Use vswhere to locate vcvarsall.bat (works for any VS version/edition) --
# identical to MsvcEnvironment.cmake, we still need the MSVC environment
# (INCLUDE/LIB/rc.exe/mt.exe) even though the compiler itself will be clang-cl.
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

if(NOT VCVARSALL OR NOT EXISTS "${VCVARSALL}")
  find_program(VCVARSALL vcvarsall.bat PATHS
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Community/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Professional/VC/Auxiliary/Build"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/18/Enterprise/VC/Auxiliary/Build"
    "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/18/BuildTools/VC/Auxiliary/Build"
    NO_DEFAULT_PATH
  )
endif()

if(NOT VCVARSALL OR NOT EXISTS "${VCVARSALL}")
  message(FATAL_ERROR "Could not find vcvarsall.bat. Install Visual Studio with the C++ workload.")
endif()

# clang-cl.exe lives under the same VS install as vcvarsall.bat, in the
# "C++ Clang Compiler for Windows" component's directory.
get_filename_component(_vs_root "${VCVARSALL}" DIRECTORY)  # .../VC/Auxiliary/Build
get_filename_component(_vs_root "${_vs_root}" DIRECTORY)   # .../VC/Auxiliary
get_filename_component(_vs_root "${_vs_root}" DIRECTORY)   # .../VC
get_filename_component(_vs_root "${_vs_root}" DIRECTORY)   # VS install root
find_program(CLANGCL_EXE clang-cl.exe
  PATHS "${_vs_root}/VC/Tools/Llvm/x64/bin"
  NO_DEFAULT_PATH
)
if(NOT CLANGCL_EXE)
  message(FATAL_ERROR "Could not find clang-cl.exe under ${_vs_root}/VC/Tools/Llvm/x64/bin. "
      "Install the \"C++ Clang Compiler for Windows\" component in Visual Studio Installer.")
endif()

message(STATUS "Running vcvarsall.bat: ${VCVARSALL}")

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

set(ENV{LIB}     "${_vc_lib}")
set(ENV{INCLUDE} "${_vc_include}")
set(ENV{PATH}    "${_vc_path}")

include_directories(SYSTEM ${_vc_include})
link_directories(${_vc_lib})

set(CMAKE_C_COMPILER   "${CLANGCL_EXE}" CACHE FILEPATH "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER "${CLANGCL_EXE}" CACHE FILEPATH "C++ compiler" FORCE)
set(CMAKE_RC_COMPILER  "${_sdk_bin}/x64/rc.exe" CACHE FILEPATH "RC compiler"  FORCE)
set(CMAKE_MT           "${_sdk_bin}/x64/mt.exe" CACHE FILEPATH "Manifest tool" FORCE)

stochfit_use_llvm_openmp("${CLANGCL_EXE}")

# clang-cl targets the MSVC ABI and links the same dynamic CRT (/MD via the
# x64-windows-static-md triplet), so the packaged app still needs the msvcp*/
# vcruntime* redistributables exactly as an MSVC build does. CMakeLists.txt reads
# these from the environment the way vcvarsall.bat would set them; we ran
# vcvarsall in a throwaway cmd subprocess above, so they never reach our process
# env unless we set them here too.
set(ENV{VCToolsInstallDir} "${_vc_tools}/")
string(REPLACE "/Tools/MSVC/" "/Redist/MSVC/" _vc_redist "${_vc_tools}")
set(ENV{VCToolsRedistDir} "${_vc_redist}/")

message(STATUS "clang-cl environment ready")
message(STATUS "  clang-cl: ${CLANGCL_EXE}")
message(STATUS "  rc: ${_sdk_bin}/x64/rc.exe")
message(STATUS "  libomp: ${OpenMP_libomp_LIBRARY}")
