cmake_minimum_required(VERSION 3.10)
CMAKE_POLICY(SET CMP0074 NEW)

# set the project name and version
project(parPT VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE "Debug")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  # set compiler debug flag
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  # set compiler all warnings flag
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
  message(ERROR "This ain't ready for release, only Debug is supported")
endif()

if (PARPT_USE_OPENMP)
  set(OMP_FLAGS "-fopenmp")
  message(STATUS "OpenMP flags are: ${OMP_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OMP_FLAGS}")
endif()

message(STATUS "CMAKE_CXX_FLAGS flags are ${CMAKE_CXX_FLAGS}")

# add the ext directory, with external TPLs
add_subdirectory(ext)

# add the source directory
add_subdirectory(src)
