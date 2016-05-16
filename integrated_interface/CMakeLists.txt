# ===========================================================================
#                  SeqAn - The Library for Sequence Analysis
# ===========================================================================
# File: /apps/bench/Interface/CMakeLists.txt
#
# CMakeLists.txt file for intgrated bench.
# ===========================================================================

cmake_minimum_required (VERSION 2.8.9)
project (seqan_apps_bench CXX)

# require c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX11_FOUND TRUE)

# compile binaries into bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------

# Search SeqAn and select dependencies.
find_package (OpenMP)
find_package (SeqAn REQUIRED)

# Was SeqAn included?
if (NOT (SEQAN_FOUND))
    message (FATAL_ERROR "SeqAn not found")
endif ()

# ----------------------------------------------------------------------------
# Build Setup
# ----------------------------------------------------------------------------

# Add include directories.
include_directories (${SEQAN_INCLUDE_DIRS})

# Add definitions set by find_package (SeqAn).
add_definitions (${SEQAN_DEFINITIONS})

# Add CXX flags found by find_package (SeqAn).
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SEQAN_CXX_FLAGS} ")

# Update the list of file names below if you add source files to your application.
add_executable(kmers_10mer_counting                        std_bench/src/kmers/kmers_10mer_counting.cpp)
add_executable(kmers_15mer_counting                        std_bench/src/kmers/kmers_15mer_counting.cpp)
add_executable(kmers_50mer_counting                        std_bench/src/kmers/kmers_50mer_counting.cpp)
add_executable(index_create                                std_bench/src/indices/index_create.cpp)
add_executable(index_exact_search_sa                       std_bench/src/indices/index_exact_search_sa.cpp)
add_executable(index_exact_search_esa                      std_bench/src/indices/index_exact_search_esa.cpp)
add_executable(index_exact_search_fm                       std_bench/src/indices/index_exact_search_fm.cpp)
add_executable(index_exact_search_wotd                     std_bench/src/indices/index_exact_search_wotd.cpp)
add_executable(index_exact_search_dfi                      std_bench/src/indices/index_exact_search_dfi.cpp)
add_executable(index_exact_search_qgram                    std_bench/src/indices/index_exact_search_qgram.cpp)
add_executable(index_one_error_approximate_search_esa      std_bench/src/indices/index_one_error_approximate_search_esa.cpp)
add_executable(index_two_errors_approximate_search_esa     std_bench/src/indices/index_two_errors_approximate_search_esa.cpp)
add_executable(index_three_errors_approximate_search_esa   std_bench/src/indices/index_three_errors_approximate_search_esa.cpp)
add_executable(pairwise_alignment_global_affine_dna        std_bench/src/pairwise_alignments/pairwise_alignment_global_affine_dna.cpp)
add_executable(pairwise_alignment_global_affine_protein    std_bench/src/pairwise_alignments/pairwise_alignment_global_affine_protein.cpp)
add_executable(pairwise_alignment_global_linear_dna        std_bench/src/pairwise_alignments/pairwise_alignment_global_linear_dna.cpp)
add_executable(pairwise_alignment_global_linear_protein    std_bench/src/pairwise_alignments/pairwise_alignment_global_linear_protein.cpp)
add_executable(pairwise_alignment_local_affine_dna         std_bench/src/pairwise_alignments/pairwise_alignment_local_affine_dna.cpp)
add_executable(pairwise_alignment_local_affine_protein     std_bench/src/pairwise_alignments/pairwise_alignment_local_affine_protein.cpp)
add_executable(pairwise_alignment_local_linear_dna         std_bench/src/pairwise_alignments/pairwise_alignment_local_linear_dna.cpp)
add_executable(pairwise_alignment_local_linear_protein     std_bench/src/pairwise_alignments/pairwise_alignment_local_linear_protein.cpp)

# Add dependencies found by find_package (SeqAn).
target_link_libraries (kmers_10mer_counting                     ${SEQAN_LIBRARIES})
target_link_libraries (kmers_15mer_counting                     ${SEQAN_LIBRARIES})
target_link_libraries (kmers_50mer_counting                     ${SEQAN_LIBRARIES})
target_link_libraries (index_create                             ${SEQAN_LIBRARIES})
target_link_libraries (index_exact_search_sa                    ${SEQAN_LIBRARIES})
target_link_libraries (index_exact_search_esa                   ${SEQAN_LIBRARIES})
target_link_libraries (index_exact_search_fm                    ${SEQAN_LIBRARIES})
target_link_libraries (index_exact_search_wotd                  ${SEQAN_LIBRARIES})
target_link_libraries (index_exact_search_dfi                   ${SEQAN_LIBRARIES})
target_link_libraries (index_exact_search_qgram                 ${SEQAN_LIBRARIES})
target_link_libraries (index_one_error_approximate_search_esa     ${SEQAN_LIBRARIES})
target_link_libraries (index_two_errors_approximate_search_esa    ${SEQAN_LIBRARIES})
target_link_libraries (index_three_errors_approximate_search_esa  ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_global_affine_dna     ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_global_affine_protein ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_global_linear_dna     ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_global_linear_protein ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_local_affine_dna      ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_local_affine_protein  ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_local_linear_dna      ${SEQAN_LIBRARIES})
target_link_libraries (pairwise_alignment_local_linear_protein  ${SEQAN_LIBRARIES})

# ----------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# CPack Install
# ----------------------------------------------------------------------------

if (SEQAN_BUILD_SYSTEM STREQUAL "APP:bench_interface")
  set (CPACK_PACKAGE_NAME "bench_interface")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "SeqAn Benchmarks Interface")
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Chenxu Pan <chenxu.pan@fu-berlin.de>")
  set (CPACK_PACKAGE_VENDOR "SeqAn Team, FU Berlin")

  seqan_configure_cpack_app (integrated_interface "bench_interface")
endif (SEQAN_BUILD_SYSTEM STREQUAL "APP:bench_interface")
