#!/bin/bash

# these are the serial kokkos/kernels
export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_clang/lib"
export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_clang/include"
# export KKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install/lib"
# export KKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install/include"
export YAMLCPP_LIBDIR="/Users/mjschm/yaml-cpp/install_clang/lib"
export YAMLCPP_INCDIR="/Users/mjschm/yaml-cpp/install_clang/include"
# use this below when compiling in serial
# -D CMAKE_CXX_COMPILER=clang++

# these are the gcc/openmp kokkos/kernels
# export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_gccomp/lib"
# export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_gccomp/include"
# export KKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_gccomp/lib"
# export KKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_gccomp/include"
# export YAMLCPP_LIBDIR="/Users/mjschm/yaml-cpp/install_gcc11/lib"
# export YAMLCPP_INCDIR="/Users/mjschm/yaml-cpp/install_gcc11/include"
# export OMP_FLAGS='-fopenmp'
# use this below when compiling for openmp
# -D CMAKE_CXX_COMPILER=g++-11

export OPENBLAS_LIBDIR="/Users/mjschm/OpenBLAS/install/lib"
export OPENBLAS_INCDIR="/Users/mjschm/OpenBLAS/install/include"
export OPENBLAS_LIBRARY=libopenblas.a

rm -rf CMake*

cmake .. \
    -D CMAKE_INSTALL_PREFIX="./install"\
    -D CMAKE_VERBOSE_MAKEFILE=ON\
    -D CMAKE_CXX_COMPILER=clang++\
    -D CMAKE_C_COMPILER=clang\
    -D CMAKE_Fortran_COMPILER=gfortran
