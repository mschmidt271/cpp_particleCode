#!/bin/bash

# choose only one of these options
# (1)
# export USE_OPENMP=false
# export USE_CUDA=false
# (2)
export USE_OPENMP=true
export USE_CUDA=false
# (3) ***NOT CURRENTLY SUPPORTED***
# export USE_OPENMP=false
# export USE_CUDA=true
# (4)
# export USE_OPENMP=true
# export USE_CUDA=true

export MACHINE=`hostname`
export HOME_DIR=$HOME
echo "Configuring for ${MACHINE}"
if [ $MACHINE = s1046231 ]; then
    export KO_LIBDIR="lib"
    if [ "$USE_OPENMP" = false ]; then
        echo "Building for serial"
        export KO_ROOT="${HOME_DIR}/kokkos/install_clang"
        export KK_ROOT="${HOME_DIR}/kokkos-kernels/install_clang"
        export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install_clang"
        export AX_ROOT="${HOME_DIR}/ArborX/install_clang"
    elif [ "$USE_OPENMP" = true ]; then
        echo "Building for OpenMP"
        export KO_ROOT="${HOME_DIR}/kokkos/install_gccomp"
        export KK_ROOT="${HOME_DIR}/kokkos-kernels/install_gccomp"
        export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install_gcc11"
        export AX_ROOT="${HOME_DIR}/ArborX/install_gccomp"
    else
        echo "ERROR: Unsupported build for this machine"
        exit
    fi
elif [ $MACHINE = s1024454 ]; then
    export KO_LIBDIR="lib64"
    if [ "$USE_OPENMP" = false ] && [ "$USE_CUDA" = false ]; then
        echo "Building for serial"
        echo "ERROR: Unsupported build for this machine"
        exit
        # export KO_ROOT="${HOME_DIR}/kokkos/"
        # export KK_ROOT="${HOME_DIR}/kokkos-kernels/"
        # export YCPP_ROOT="${HOME_DIR}/yaml-cpp/"
        # export AX_ROOT="${HOME_DIR}/ArborX/"
    elif [ "$USE_OPENMP" = true ] && [ "$USE_CUDA" = false ]; then
        echo "Building for OpenMP without CUDA"
        export KO_ROOT="${HOME_DIR}/kokkos/install_omp_only"
        export KK_ROOT="${HOME_DIR}/kokkos-kernels/install_omp_only"
        export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install"
        export AX_ROOT="${HOME_DIR}/ArborX/install_gccomp"
    elif [ "$USE_OPENMP" = false ] && [ "$USE_CUDA" = true ]; then
        echo "Building without OpenMP"
        echo "ERROR: Unsupported build for this machine"
        exit
        # export DEVICE_ARCH="MAXWELL52"
        # export KO_ROOT="${HOME_DIR}/kokkos/"
        # export KK_ROOT="${HOME_DIR}/kokkos-kernels/"
        # export YCPP_ROOT="${HOME_DIR}/yaml-cpp/"
        # export AX_ROOT="${HOME_DIR}/ArborX/"
    elif [ "$USE_OPENMP" = true ] && [ "$USE_CUDA" = true ]; then
        echo "Building for OpenMP and CUDA"
        export DEVICE_ARCH="MAXWELL52"
        export KO_ROOT="${HOME_DIR}/kokkos/install"
        export KK_ROOT="${HOME_DIR}/kokkos-kernels/install"
        export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install"
        export AX_ROOT="${HOME_DIR}/ArborX/install_cuda"
    else
        echo "ERROR: Unsupported build for this machine"
        exit
    fi
elif [ $MACHINE = clamps ]; then
    export HOME_DIR="${HOME}/mjschmidt"
    export KO_LIBDIR="lib"
    if [ "$USE_OPENMP" = false ] && [ "$USE_CUDA" = false ]; then
        echo "Building for serial"
        echo "ERROR: Unsupported build for this machine"
        exit
        # export KO_ROOT="${HOME_DIR}/kokkos/"
        # export KK_ROOT="${HOME_DIR}/kokkos-kernels/"
        # export YCPP_ROOT="${HOME_DIR}/yaml-cpp/"
        # export AX_ROOT="${HOME_DIR}/ArborX/"
    elif [ "$USE_OPENMP" = true ] && [ "$USE_CUDA" = false ]; then
        echo "Building for OpenMP without CUDA"
        export KO_ROOT="${HOME_DIR}/kokkos/install_omp"
        export KK_ROOT="${HOME_DIR}/kokkos-kernels/install_omp"
        export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install"
        export AX_ROOT="${HOME_DIR}/ArborX/install_omp"
    elif [ "$USE_OPENMP" = false ] && [ "$USE_CUDA" = true ]; then
        echo "Building without OpenMP"
        echo "ERROR: Unsupported build for this machine"
        exit
        # export DEVICE_ARCH="TURING75"
        # export KO_ROOT="${HOME_DIR}/kokkos/"
        # export KK_ROOT="${HOME_DIR}/kokkos-kernels/"
        # export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install"
        # export AX_ROOT="${HOME_DIR}/ArborX/"
    elif [ "$USE_OPENMP" = true ] && [ "$USE_CUDA" = true ]; then
        echo "Building for OpenMP and CUDA"
        export DEVICE_ARCH="TURING75"
        export KO_ROOT="${HOME_DIR}/kokkos/install_cuda"
        export KK_ROOT="${HOME_DIR}/kokkos-kernels/install_cuda"
        export YCPP_ROOT="${HOME_DIR}/yaml-cpp/install"
        export AX_ROOT="${HOME_DIR}/ArborX/install_cuda"
    else
        echo "ERROR: Unsupported build for this machine"
        exit
    fi
else
    "Unrecognized machine--atempting default build (this may not work)"
fi
echo "Home directory is ${HOME_DIR}"

# change these to the compilers you plan to use
# note that cuda builds require the CXX compiler to be the
# kokkos-provided nvcc_wrapper
export MAC_SERIAL_CPP="clang++"
export MAC_SERIAL_C="clang"
export MAC_OMP_CPP="g++-11"
export MAC_OMP_C="gcc-11"
export LINUX_CPP="mpicxx"
export LINUX_C="mpicc"

# set these manually to where they are on your machine, or just comment out
export KOKKOS_LIBDIR="${KO_ROOT}/${KO_LIBDIR}"
export KOKKOS_INCDIR="${KO_ROOT}/include"
export KOKKOS_LIBRARY="libkokkoscore.a"
export KOKKOSKERNELS_LIBDIR="${KK_ROOT}/${KO_LIBDIR}"
export KOKKOSKERNELS_INCDIR="${KK_ROOT}/include"
export KOKKOSKERNELS_LIBRARY="libkokkoskernels.a"
export YAML_CPP_LIBDIR="${YCPP_ROOT}/lib"
export YAML_CPP_INCDIR="${YCPP_ROOT}/include"
export YAML_CPP_LIBRARY="libyaml-cpp.a"
export ARBORX_INCDIR="${AX_ROOT}/include"

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
            export CXX=${HOME_DIR}/kokkos/bin/nvcc_wrapper
        fi
    else
        export CXX=$LINUX_CPP
    fi
    export CC=$LINUX_C
else
    if [ "$USE_OPENMP" = true ]
    # change these to your compilers
    then
        export CXX=$MAC_OMP_CPP
        export CC=$MAC_OMP_CPP
    else
        export CXX=$MAC_SERIAL_CPP
        export CC=$MAC_SERIAL_C
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
