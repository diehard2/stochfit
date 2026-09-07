if(EXISTS "${CURRENT_INSTALLED_DIR}/share/lapack-reference/copyright")
    message(FATAL_ERROR "Can't build ${PORT} if lapack-reference is installed. Please remove lapack-reference:${TARGET_TRIPLET}, and try to install ${PORT}:${TARGET_TRIPLET} again.")
endif()

vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
  URLS "https://www.netlib.org/clapack/clapack-3.2.1-CMAKE.tgz"
  FILENAME "clapack-3.2.1.tgz"
  SHA512 cf19c710291ddff3f6ead7d86bdfdeaebca21291d9df094bf0a8ef599546b007757fb2dbb19b56511bb53ef7456eac0c73973b9627bf4d02982c856124428b49
)

vcpkg_extract_source_archive(
  SOURCE_PATH
  ARCHIVE "${ARCHIVE}"
  PATCHES
      remove_internal_blas.patch
      fix-ConfigFile.patch
      fix-install.patch
      support-uwp.patch
      fix-integer-typedef.patch
)

set(ARITH_PATH)
if(DEFINED CLAPACK_ARITH_PATH)
  set(ARITH_PATH "-DARITH_PATH=${CLAPACK_ARITH_PATH}")
elseif(NOT TARGET_TRIPLET STREQUAL HOST_TRIPLET)
  if(VCPKG_TARGET_IS_WINDOWS OR VCPKG_TARGET_IS_UWP)
    if(VCPKG_TARGET_ARCHITECTURE MATCHES "^x64$|^arm64$")
      set(ARITH_PATH "-DARITH_PATH=${CMAKE_CURRENT_LIST_DIR}/arith_win64.h")
    else()
      set(ARITH_PATH "-DARITH_PATH=${CMAKE_CURRENT_LIST_DIR}/arith_win32.h")
    endif()
  elseif(VCPKG_TARGET_IS_OSX OR VCPKG_TARGET_IS_IOS)
    set(ARITH_PATH "-DARITH_PATH=${CMAKE_CURRENT_LIST_DIR}/arith_osx.h")
  elseif(VCPKG_TARGET_IS_LINUX AND VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    set(ARITH_PATH "-DARITH_PATH=${CMAKE_CURRENT_LIST_DIR}/arith_linux64.h")
  else()
    message(WARNING
"Unable to cross-compile clapack for ${VCPKG_TARGET_ARCHITECTURE}-${VCPKG_CMAKE_SYSTEM_NAME}.
No arith.h is available and arithchk must be executed for the target.
To fix this issue, define CLAPACK_ARITH_PATH in your triplet to the location of a pre-generated arith.h file.

Continuing with trying to run arithchk anyway.")
  endif()
endif()

set(C_STANDARD_OPTIONS)
if(VCPKG_TARGET_IS_LINUX)
  # f2c-generated code (e.g. SRC/sgees.c) declares old-style unprototyped
  # function-pointer typedefs and calls them with arguments — legal K&R/C17
  # ("()" means "unspecified parameters"), but GCC 15 defaults to gnu23,
  # where "()" means "takes no parameters," turning every such call into a
  # hard error ("too many arguments to function 'select'"). Pin the C
  # dialect back to gnu17 so this 20+ year old Fortran-to-C output still
  # builds under new toolchains.
  set(C_STANDARD_OPTIONS -DCMAKE_C_STANDARD=17 -DCMAKE_C_STANDARD_REQUIRED=ON -DCMAKE_C_EXTENSIONS=ON)
endif()

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    ${ARITH_PATH}
    ${C_STANDARD_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

#TODO: fix the official exported targets, since they are broken (luckily it seems that no-one uses them for now)
vcpkg_cmake_config_fixup(CONFIG_PATH share/clapack)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Install clapack wrappers.
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/vcpkg-cmake-wrapper.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}/wrapper")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/FindLAPACK.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
