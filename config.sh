#!/bin/bash

export KOKKOS_LIBDIR="/Users/mjschm/kokkos/install/lib"
export KOKKOS_INCDIR="/Users/mjschm/kokkos/install/include"

export KKERNELS_LIBDIR="/Users/mjschm/kokkos-kernels/install/lib"
export KKERNELS_INCDIR="/Users/mjschm/kokkos-kernels/install/include"

export OPENBLAS_LIBDIR="/usr/local/opt/openblas/lib"
export OPENBLAS_INCDIR="/usr/local/opt/openblas/include"

rm -rf CMake*

cmake .. \
    -D CMAKE_INSTALL_PREFIX="./install"
