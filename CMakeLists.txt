# ===========================================================================
#                  SeqAn - The Library for Sequence Analysis
# ===========================================================================
# File: /apps/bench/CMakeLists.txt
#
# CMakeLists.txt file for bench.
# ===========================================================================

cmake_minimum_required (VERSION 2.8.9)
project (seqan_apps_bench)
message (STATUS "Configuring apps/bench")

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------

# Search C++11.
find_package(CXX11 REQUIRED)

if (NOT CXX11_FOUND)
    message (STATUS "  C++11 compiler required to build bench.")
    return ()
endif (NOT CXX11_FOUND)

# Search SeqAn and select dependencies.
set (SEQAN_FIND_DEPENDENCIES OpenMP ZLIB BZIP2)
find_package (SeqAn REQUIRED)

# Warn if Zlib was not found.
if (NOT SEQAN_HAS_OPENMP)
  message (STATUS "  OpenMP not found: building bench without multithreading.")
endif (NOT SEQAN_HAS_OPENMP)

# Warn if Zlib was not found.
if (NOT SEQAN_HAS_ZLIB)
  message (STATUS "  Zlib not found: building bench without gzip decompression.")
endif (NOT SEQAN_HAS_ZLIB)

# Warn if Bzip2 was not found.
if (NOT SEQAN_HAS_BZIP2)
  message (STATUS "  Bzip2 not found: building bench without bzip2 decompression.")
endif (NOT SEQAN_HAS_BZIP2)

# ----------------------------------------------------------------------------
# Arch Setup
# ----------------------------------------------------------------------------

if (CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)
  set (SEQAN_CXX_FLAGS "${SEQAN_CXX_FLAGS} -march=native")
endif (CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)

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
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SEQAN_CXX_FLAGS} ${CXX11_CXX_FLAGS}")

# Update the list of file names below if you add source files to your application.
add_executable(bench_dump dump.cpp options.h types.h run.h misc.h)
add_executable(bench_construct construct.cpp options.h types.h run.h misc.h)
add_executable(bench_visit visit.cpp options.h types.h run.h misc.h)
add_executable(bench_query query.cpp options.h types.h run.h misc.h find_backtracking_dfs.h find_backtracking_bfs.h)
add_executable(bench_filter filter.cpp options.h types.h run.h misc.h verifier.h)

# Add dependencies found by find_package (SeqAn).
target_link_libraries (bench_dump ${SEQAN_LIBRARIES})
target_link_libraries (bench_construct ${SEQAN_LIBRARIES})
target_link_libraries (bench_visit ${SEQAN_LIBRARIES})
target_link_libraries (bench_query ${SEQAN_LIBRARIES})
target_link_libraries (bench_filter ${SEQAN_LIBRARIES})

# ----------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------

# Set variables for installing, depending on the selected build type.
if (NOT SEQAN_PREFIX_SHARE_DOC)
  seqan_setup_install_vars (bench_dump bench_construct bench_visit bench_query bench_filter)
endif (NOT SEQAN_PREFIX_SHARE_DOC)

# Install bench in ${PREFIX}/bin directory
install (TARGETS bench_dump bench_construct bench_visit bench_query bench_filter
         DESTINATION bin)

# Install non-binary files for the package to "." for app builds and
# ${PREFIX}/share/doc/bench for SeqAn release builds.
install (FILES LICENSE README
         DESTINATION ${SEQAN_PREFIX_SHARE_DOC})

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
add_subdirectory(integrated_interface)
# ----------------------------------------------------------------------------
# CPack Install
# ----------------------------------------------------------------------------

if (SEQAN_BUILD_SYSTEM STREQUAL "APP:bench")
  set (CPACK_PACKAGE_NAME "bench")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "SeqAn Benchmarks")
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Enrico Siragusa <enrico.siragusa@fu-berlin.de>")
  set (CPACK_PACKAGE_VENDOR "SeqAn Team, FU Berlin")

  seqan_configure_cpack_app (bench "bench")
endif (SEQAN_BUILD_SYSTEM STREQUAL "APP:bench")
