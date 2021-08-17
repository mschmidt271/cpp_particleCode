#!/bin/bash

# these are the serial kokkos/kernels
# export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_clang/lib"
# export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_clang/include"
# export KOKKOS_LIBRARY="libkokkoscore.a"
# export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_clang_noblas/lib"
# export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_clang_noblas/include"
# export KOKKOSKERNELS_LIBRARY="libkokkoskernels.a"
# export YAML_CPP_LIBDIR="/Users/mjschm/yaml-cpp/install_clang/lib"
# export YAML_CPP_INCDIR="/Users/mjschm/yaml-cpp/install_clang/include"
# export YAML_CPP_LIBRARY="libyaml-cpp.a"
# use this below when compiling in serial
# -D CMAKE_CXX_COMPILER=clang++

# these are the gcc/openmp kokkos/kernels
# export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_gccomp/lib"
# export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_gccomp/include"
# export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_gccomp/lib"
# export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_gccomp/include"
# export YAML_CPP_LIBDIR="/Users/mjschm/yaml-cpp/install_gcc11/lib"
# export YAML_CPP_INCDIR="/Users/mjschm/yaml-cpp/install_gcc11/include"
# use this below when compiling for openmp
# -D CMAKE_CXX_COMPILER=g++-11

export USE_OPENMP=True

if $USE_OPENMP
then
    export CXX=g++-11
    export CC=gcc-11
else
    export CXX=clang++
    export CC=clang
fi

rm -rf CMake*

cmake .. \
    -D CMAKE_INSTALL_PREFIX="./"\
    -D CMAKE_VERBOSE_MAKEFILE=ON\
    -D CMAKE_CXX_COMPILER=$CXX\
    -D CMAKE_C_COMPILER=$CC\
    -D PARPT_USE_OPENMP=$USE_OPENMP
