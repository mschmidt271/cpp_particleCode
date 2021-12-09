#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=8
export OMP_PROC_BIND=spread

export KK_TOOLS_DIR=/home/pfsuser/mjschmidt/kokkos-tools

# kernel logger
export KOKKOS_PROFILE_LIBRARY=${KK_TOOLS_DIR}/kp_kernel_logger.so
export PATH=${PATH}:${KK_TOOLS_DIR}

# run the program and redirect the error output
./../../bin/parPT /tests/RW_only/RW_only_input.yaml -v 2> a.err
# echo "c"
# ./bin/parPT /data/particleParams.yaml -v
# cd plotting
# python3 plotParticles.py3
# cd ..
