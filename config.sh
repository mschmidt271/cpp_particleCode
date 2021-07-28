#!/bin/bash

# these are the serial kokkos/kernels
export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_clang/lib"
export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_clang/include"
export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_clang_noblas/lib"
export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_clang_noblas/include"
export KOKKOSKERNELS_LIBRARY="/Users/mjschm/kokkos-kernels/install_clang_noblas/lib/libkokkoskernels.a"
export YAMLCPP_LIBDIR="/Users/mjschm/yaml-cpp/install_clang/lib"
export YAMLCPP_INCDIR="/Users/mjschm/yaml-cpp/install_clang/include"
# use this below when compiling in serial
# -D CMAKE_CXX_COMPILER=clang++

# these are the gcc/openmp kokkos/kernels
# export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_gccomp/lib"
# export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_gccomp/include"
# export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_gccomp/lib"
# export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_gccomp/include"
# export YAMLCPP_LIBDIR="/Users/mjschm/yaml-cpp/install_gcc11/lib"
# export YAMLCPP_INCDIR="/Users/mjschm/yaml-cpp/install_gcc11/include"
# export OMP_FLAGS='-fopenmp'
# use this below when compiling for openmp
# -D CMAKE_CXX_COMPILER=g++-11

rm -rf CMake*

cmake .. \
    -D CMAKE_INSTALL_PREFIX="./install"\
    -D CMAKE_VERBOSE_MAKEFILE=ON\
    -D CMAKE_CXX_COMPILER=clang++\
    -D CMAKE_C_COMPILER=clang
