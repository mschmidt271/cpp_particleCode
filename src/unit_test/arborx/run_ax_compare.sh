#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=12
export OMP_PROC_BIND=spread
# export OMP_PLACES=threads

export dim=(1 1 1)
export N=(1000 1000 1000)
export L=(10 10 10)
export dist=(1.1345 1.3 1.6)
export pt_type=("rand" "rand" "rand")
export infile=("test_pts.yaml" "test_pts.yaml" "test_pts.yaml")

# kernel logger
# export KOKKOS_PROFILE_LIBRARY=${HOME}/kokkos-tools/kp_kernel_logger.so
# export PATH=${PATH}:${HOME}/kokkos-tools/

for ((i=0; i<=2; i++))
do
    echo "==========================================="

    echo "num_threads = ${OMP_NUM_THREADS}"
    echo "dim =  ${dim[i]}"
    echo "N =  ${N[i]}"
    echo "L =  ${L[i]}"
    echo "dist =  ${dist[i]}"
    echo "pt_type =  ${pt_type[i]}"
    echo "infile =  ${infile[i]}"

    echo "==========================================="

    ./gen_pts.py3 ${dim[i]} ${N[i]} ${L[i]} ${dist[i]} ${pt_type[i]} --fname=${infile[i]}
    ./AX_compare ${infile[i]}
    # ./AX_compare ${infile[i]} > a.out 2> a.err
done
