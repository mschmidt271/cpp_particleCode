#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=8
export OMP_PROC_BIND=spread
# export OMP_PLACES=cores

# simple kernel timer location
# export simple_kt=true
# export KOKKOS_PROFILE_LIBRARY=/Users/mjschm/kokkos-tools/profiling/simple-kernel-timer/kp_kernel_timer.so
# export PATH=${PATH}:/Users/mjschm/kokkos-tools/profiling/

# space time stack
# export st_stack=true
# export KOKKOS_PROFILE_LIBRARY=/Users/mjschm/kokkos-tools/profiling/space-time-stack/kp_space_time_stack.so
# export PATH=${PATH}:/Users/mjschm/kokkos-tools/profiling/

# memory events
# export KOKKOS_PROFILE_LIBRARY=/Users/mjschm/kokkos-tools/profiling/memory-events/kp_memory_events.so
# export PATH=${PATH}:/Users/mjschm/kokkos-tools/profiling/

# kernel logger
# export KOKKOS_PROFILE_LIBRARY=/Users/mjschm/kokkos-tools/debugging/kernel-logger/kp_kernel_logger.so
# export PATH=${PATH}:/Users/mjschm/kokkos-tools/debugging/


# run the program and redirect the error output
# ./bin/parPT /data/particleParams.yaml -v 2> data/a.err
# ./bin/parPT /data/particleParams.yaml -v
# cd plotting
# python3 plotParticles.py3
# cd ..
# run the program and redirect screen and error output
# ./parPT.exe > a.out 2> a.err

if [ "$st_stack" = true ]
then
    ./bin/parPT /data/particleParams.yaml -v > prof_results.txt
    subl prof_results.txt
    # echo "a"
elif [ "$simple_kt" = true ]
then
    ./bin/parPT /data/particleParams.yaml -v
    kp_reader *.dat > prof_results.txt
    # echo "b"
    rm *.dat
    subl prof_results.txt
else
    # run the program and redirect the error output
    ./bin/parPT /data/particleParams.yaml -v 2> data/a.err
    # echo "c"
    # ./bin/parPT /data/particleParams.yaml -v
    cd plotting
    python3 plotParticles.py3
    cd ..
    # run the program and redirect screen and error output
    # ./parPT.exe > a.out 2> a.err
fi

# run profiler and open results
# rm *.dat
# ./bin/parPT /data/particleParams.yaml -v
# kp_reader *.dat > prof_results.txt
# subl ststack_results.txt
