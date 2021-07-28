# cpp_particleCode

This repository contains a basic random walk particle code, written in C++.

As it stands, the top of the main script includes a toy example of constructing a sparse matrix using Kokkos Kernels and conducting a matrix-vector multiply, also using Kokkos Kernels.

Note that simply downloading the repository will not work with the external projects (Kokkos, Kokkos Kernels, yaml-cpp), as they are git submodules.
As such, the repository must be cloned.
For example, in whichever directory you want to put the code:

1. `git clone --recurse-submodules -j8 git@github.com:mschmidt271/cpp_particleCode.git`
    - Note: the -j8 is a parallel flag, allowing git to fetch up to 8 submodules in parallel.
1. `cd cpp_particleCode`

- To build (and run):
    1. `mkdir build && cd build`
    1. `../config.sh`
    1. `make install`
    1. `cd install`
    1. `./run.sh`
