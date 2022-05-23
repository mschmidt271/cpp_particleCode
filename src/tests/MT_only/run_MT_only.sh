#!/bin/bash

# OpenMP environment variables
if [ "$IN_CONTAINER" = true ]; then
    export OMP_NUM_THREADS=$(nproc)
else
    export OMP_NUM_THREADS=12
fi
export OMP_PROC_BIND=spread
export OMP_PLACES=threads

# kernel logger
export KOKKOS_PROFILE_LIBRARY=${HOME}/kokkos-tools/kp_kernel_logger.so
export PATH=${PATH}:${HOME}/kokkos-tools/

export PT_EXE=./../../bin/parPT
export YAML_IN=/tests/MT_only/data/MT_only_input.yaml
export YAML_PTS=../MT_only/data/MT_only_input.yaml

./../utils/gen_pts.py3 --fname=${YAML_PTS}

# run the program and redirect the error output
$PT_EXE $YAML_IN -v > a.out 2> a.err
