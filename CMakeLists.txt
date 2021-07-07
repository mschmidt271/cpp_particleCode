cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(ParticleRW VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set compiler debug flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# set compiler all warnings flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# add the source directory
add_subdirectory(src)
