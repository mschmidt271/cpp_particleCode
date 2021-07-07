#!/bin/bash

# run the program and redirect the error output
./bin/particleRW -v 2> data/a.err
cd plotting
python3 plotParticles.py3
cd ..
# run the program and redirect screen and error output
# ./particleRW.exe > a.out 2> a.err
