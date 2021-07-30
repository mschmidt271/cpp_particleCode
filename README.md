# cpp_particleCode

This repository contains a basic random walk particle code, written in C++.

The only current dependencies are __python3__, with __numpy__ and __matplotlib__ (should be installed on Mac, but `brew install python`, `pip install numpy`, and `pip install matplotlib` should get you there if not).

As it stands, the top of the main script includes a toy example of constructing a sparse matrix using Kokkos Kernels and conducting a matrix-vector multiply, also using Kokkos Kernels.

Note that simply downloading the repository will not work with the external projects (Kokkos, Kokkos Kernels, yaml-cpp), as they are git submodules.
As such, the repository must be cloned.
For example, in whichever directory you want to put the code:

1. Clone the repository:
    - If you use https (this is the case if you haven't set up a [github ssh key](https://docs.github.com/en/github/authenticating-to-github/connecting-to-github-with-ssh)):
        - `git clone --recurse-submodules -j8 https://github.com/mschmidt271/cpp_particleCode.git`
    - If you use ssh:
        - `git clone --recurse-submodules -j8 git@github.com:mschmidt271/cpp_particleCode.git`
    - Note: the `-j8` is a parallel flag, allowing git to fetch up to 8 submodules in parallel.
1. `cd cpp_particleCode`

- To build (and run):
    1. `mkdir build && cd build`
    1. `../config.sh`
        - If you're savvy and want to pre-install the libraries that are commented in `config.sh` to cut down on build times, follow the given templates to specify their paths.
    1. `make -j install`
        - Note: similarly to above, the `-j` flag executes the make using the max number of cores available.
    1. `cd install`
    1. `./run.sh`
