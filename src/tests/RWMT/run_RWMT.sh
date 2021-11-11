#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=8
export OMP_PROC_BIND=spread

# run the program and redirect the error output
./../../bin/parPT /tests/RWMT/RWMT_input.yaml -v 2> a.err
# echo "c"
# ./bin/parPT /data/particleParams.yaml -v
# cd plotting
# python3 plotParticles.py3
# cd ..
