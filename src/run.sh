#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=8
export OMP_PROC_BIND=spread
# export OMP_PLACES=cores

# run the program and redirect the error output
# ./bin/parPT /data/particleParams.yml -v 2> data/a.err
./bin/parPT /data/particleParams.yml -v
cd plotting
python3 plotParticles.py3
cd ..
# run the program and redirect screen and error output
# ./parPT.exe > a.out 2> a.err
