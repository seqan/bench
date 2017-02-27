# ===========================================================================
# CMakeLists.txt file for validators.
# ===========================================================================

cmake_minimum_required (VERSION 3.0.0)
project (seqan_apps_bench CXX)

# require c++11
if (CMAKE_VERSION GREATER 3.1.0)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CXX11_FOUND TRUE)
else ()
  set(CMAKE_CXX_FLAGS "-std=c++11")
endif ()

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

# Set Release mode
set(CMAKE_BUILD_TYPE "Release")

# Add include directories.
include_directories (${SEQAN_INCLUDE_DIRS})

# Add definitions set by find_package (SeqAn).
add_definitions (${SEQAN_DEFINITIONS})

# Add CXX flags found by find_package (SeqAn).
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SEQAN_CXX_FLAGS} ")

# ----------------------------------------------------------------------------
# Install python scripts
# ----------------------------------------------------------------------------
find_package(PythonInterp REQUIRED)

if (WIN32)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} setup.py py2exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
else()
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} setup.py build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
