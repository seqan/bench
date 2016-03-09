# ===========================================================================
#                  SeqAn - The Library for Sequence Analysis
# ===========================================================================
# File: /apps/bench/Interface/CMakeLists.txt
#
# CMakeLists.txt file for intgrated bench.
# ===========================================================================

cmake_minimum_required (VERSION 2.8.9)
project (seqan_apps_bench CXX)
message (STATUS "Configuring apps/bench/Integrated_Interface/std_bench")

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------

# Search SeqAn and select dependencies.
find_package (SeqAn REQUIRED)

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
add_executable(std_bench_global_alignment_dna std_bench/pa_global.cpp)

# Add dependencies found by find_package (SeqAn).
target_link_libraries (std_bench_global_alignment_dna ${SEQAN_LIBRARIES})

# ----------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------

# Install bench in ${PREFIX}/bin directory
install (TARGETS std_bench_global_alignment_dna
         DESTINATION bin)

# ----------------------------------------------------------------------------
# CPack Install
# ----------------------------------------------------------------------------

if (SEQAN_BUILD_SYSTEM STREQUAL "APP:bench_interface")
  set (CPACK_PACKAGE_NAME "bench_interface")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "SeqAn Benchmarks Interface")
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Enrico Siragusa <enrico.siragusa@fu-berlin.de>")
  set (CPACK_PACKAGE_VENDOR "SeqAn Team, FU Berlin")

  seqan_configure_cpack_app (integrated_interface "bench_interface")
endif (SEQAN_BUILD_SYSTEM STREQUAL "APP:bench_interface")


