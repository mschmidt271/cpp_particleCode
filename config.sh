#!/bin/bash

# these are the serial kokkos/kernels
# export USE_OPENMP=false
# export USE_CUDA=false
# export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_clang/lib"
# export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_clang/include"
# export KOKKOS_LIBRARY="libkokkoscore.a"
# export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_clang/lib"
# export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_clang/include"
# export KOKKOSKERNELS_LIBRARY="libkokkoskernels.a"
# export YAML_CPP_LIBDIR="/Users/mjschm/yaml-cpp/install_clang/lib"
# export YAML_CPP_INCDIR="/Users/mjschm/yaml-cpp/install_clang/include"
# export YAML_CPP_LIBRARY="libyaml-cpp.a"

# these are the gcc/openmp kokkos/kernels
# export USE_OPENMP=true
# export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install_gccomp/lib"
# export KOKKOS_INCDIR="/Users/mjschm/kokkos/install_gccomp/include"
# export KOKKOS_LIBRARY="libkokkoscore.a"
# export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_gccomp/lib"
# export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_gccomp/include"
# export KOKKOSKERNELS_LIBRARY="libkokkoskernels.a"
# export YAML_CPP_LIBDIR="/Users/mjschm/yaml-cpp/install_gcc11/lib"
# export YAML_CPP_INCDIR="/Users/mjschm/yaml-cpp/install_gcc11/include"
# export YAML_CPP_LIBRARY="libyaml-cpp.a"

# these are the cuda kokkos/kernels
# export USE_OPENMP=false
# export USE_CUDA=true
# export KOKKOS_LIBDIR="/home/mjschm/kokkos/install/lib64"
# export KOKKOS_INCDIR="/home/mjschm/kokkos/install/include"
# export KOKKOS_LIBRARY="libkokkoscore.a"
# export BUILD_KOKKOS=false
# export KOKKOSKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install_gccomp/lib"
# export KOKKOSKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install_gccomp/include"
# export KOKKOSKERNELS_LIBRARY="libkokkoskernels.a"
# export YAML_CPP_LIBDIR="/Users/mjschm/yaml-cpp/install_gcc11/lib"
# export YAML_CPP_INCDIR="/Users/mjschm/yaml-cpp/install_gcc11/include"
# export YAML_CPP_LIBRARY="libyaml-cpp.a"

# if we are building kokkos as an external project
export USE_OPENMP=false
export USE_CUDA=true
export BUILD_KOKKOS=true
export DEVICE_ARCH="MAXWELL52"
export YAML_CPP_LIBDIR="/Users/mjschm/yaml-cpp/install_gcc11/lib"
export YAML_CPP_INCDIR="/Users/mjschm/yaml-cpp/install_gcc11/include"
export YAML_CPP_LIBRARY="libyaml-cpp.a"

export OS=`uname -s`
if [ "$OS" == "Linux" ]
then
    if [ "$USE_CUDA" = true ]
    then
        if [ "$BUILD_KOKKOS" = true ]
        then
            export CXX=`pwd`/../ext/kokkos/bin/nvcc_wrapper
            echo $CXX
            export GPU_DEVICE_ARCH_FLAG="-DKokkos_ARCH_"$DEVICE_ARCH"=ON"
            echo $GPU_DEVICE_ARCH_FLAG
        else
            export CXX=$HOME/kokkos/bin/nvcc_wrapper
        fi
    else
        export CXX=g++
    fi
    export CC=gcc
else
    if [ "$USE_OPENMP" = true ]
    then
        export CXX=g++-11
        export CC=gcc-11
    else
        export CXX=clang++
        export CC=clang
    fi
fi

rm -rf CMake*

cmake .. \
    -D CMAKE_INSTALL_PREFIX="./"\
    -D CMAKE_VERBOSE_MAKEFILE=ON\
    -D CMAKE_CXX_COMPILER=$CXX\
    -D CMAKE_C_COMPILER=$CC\
    -D PARPT_USE_OPENMP=$USE_OPENMP\
    -D PARPT_USE_CUDA=$USE_CUDA
