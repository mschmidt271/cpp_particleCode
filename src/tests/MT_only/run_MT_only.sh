#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=8
export OMP_PROC_BIND=spread

# kernel logger
export KOKKOS_PROFILE_LIBRARY=${HOME}/kokkos-tools/kp_kernel_logger.so
export PATH=${PATH}:${HOME}/kokkos-tools/

# run the program and redirect the error output
# ./../../bin/parPT /tests/MT_only/MT_only_input.yaml -v 2> a.err
./../../bin/parPT /tests/MT_only/MT_only_input.yaml -v > a.out 2> a.err
# echo "c"
# ./bin/parPT /data/particleParams.yaml -v
# cd plotting
# python3 plotParticles.py3
# cd ..
