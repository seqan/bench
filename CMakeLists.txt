# ===========================================================================
#                  SeqAn - The Library for Sequence Analysis
# ===========================================================================
# File: /core/apps/ibench/CMakeLists.txt
#
# CMakeLists.txt file for ibench.
# ===========================================================================

cmake_minimum_required (VERSION 2.8.9)
project (seqan_core_apps_ibench)
message (STATUS "Configuring core/apps/ibench")

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------

# Search SeqAn and select dependencies.
set (SEQAN_FIND_DEPENDENCIES ZLIB BZIP2)
find_package (SeqAn REQUIRED)

# Warn if Zlib was not found.
if (NOT SEQAN_HAS_ZLIB)
  message (STATUS "  Zlib not found: building ibench without gzip decompression.")
endif (NOT SEQAN_HAS_ZLIB)

# Warn if BZip2 was not found.
if (NOT SEQAN_HAS_BZIP2)
  message (STATUS "  BZip2 not found: building ibench without bzip2 decompression.")
endif (NOT SEQAN_HAS_BZIP2)

# ----------------------------------------------------------------------------
# Arch Setup
# ----------------------------------------------------------------------------

set (SEQAN_CXX_FLAGS "${SEQAN_CXX_FLAGS} -march=corei7")

if (CMAKE_COMPILER_IS_GNUCXX)
  set (SEQAN_CXX_FLAGS "${SEQAN_CXX_FLAGS} -mpopcnt")
endif (CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_COMPILER_IS_GNUCXX AND APPLE)
  set (SEQAN_CXX_FLAGS "${SEQAN_CXX_FLAGS} -mno-avx")
endif (CMAKE_COMPILER_IS_GNUCXX AND APPLE)

# ----------------------------------------------------------------------------
# Build Setup
# ----------------------------------------------------------------------------

# Add include directories.
include_directories (${SEQAN_INCLUDE_DIRS})

# Add definitions set by find_package (SeqAn).
add_definitions (${SEQAN_DEFINITIONS})

# Add definitions set by the build system.
add_definitions (-DSEQAN_REVISION="${SEQAN_REVISION}")
add_definitions (-DSEQAN_DATE="${SEQAN_DATE}")

# Add CXX flags found by find_package (SeqAn).
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SEQAN_CXX_FLAGS}")

# Update the list of file names below if you add source files to your application.
add_executable(ibench_dump dump.cpp options.h types.h run.h)
add_executable(ibench_construct construct.cpp options.h types.h run.h)
add_executable(ibench_query query.cpp options.h types.h run.h)

# Add dependencies found by find_package (SeqAn).
target_link_libraries (ibench_dump ${SEQAN_LIBRARIES})
target_link_libraries (ibench_construct ${SEQAN_LIBRARIES})
target_link_libraries (ibench_query ${SEQAN_LIBRARIES})

# ----------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------

# Set variables for installing, depending on the selected build type.
if (NOT SEQAN_PREFIX_SHARE_DOC)
  seqan_setup_install_vars (ibench_dump ibench_construct ibench_query)
endif (NOT SEQAN_PREFIX_SHARE_DOC)

# Install ibench in ${PREFIX}/bin directory
install (TARGETS ibench_dump ibench_construct ibench_query
         DESTINATION bin)

# Install non-binary files for the package to "." for app builds and
# ${PREFIX}/share/doc/ibench for SeqAn release builds.
install (FILES LICENSE README
         DESTINATION ${SEQAN_PREFIX_SHARE_DOC})

# ----------------------------------------------------------------------------
# CPack Install
# ----------------------------------------------------------------------------

if (SEQAN_BUILD_SYSTEM STREQUAL "APP:ibench")
  set (CPACK_PACKAGE_NAME "ibench")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "iBench - Full-text indexing benchmarks")
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Enrico Siragusa <enrico.siragusa@fu-berlin.de>")
  set (CPACK_PACKAGE_VENDOR "SeqAn Team, FU Berlin")

  seqan_configure_cpack_app (ibench "ibench")
endif (SEQAN_BUILD_SYSTEM STREQUAL "APP:ibench")
