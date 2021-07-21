cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(ParticleRW VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set compiler debug flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# set compiler all warnings flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (DEFINED ENV{OMP_FLAGS})
  message(STATUS "OpenMP flags are $ENV{OMP_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{OMP_FLAGS}")
endif()

message(STATUS "CMAKE_CXX_FLAGS flags are ${CMAKE_CXX_FLAGS}")

# add the source directory
add_subdirectory(src)
