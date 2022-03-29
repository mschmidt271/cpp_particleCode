#!/bin/bash

# OpenMP environment variables
export OMP_NUM_THREADS=12
export OMP_PROC_BIND=spread
export OMP_PLACES=cores

# whether we are on a remote computer and don't want to plot
# FIXME: change this to string matching so we only need one field
export simple_kt=false
export plot=true

export MACHINE=`hostname`
export HOME_DIR=$HOME
if [ $MACHINE = s1046231 ]; then
    export KK_TOOLS_DIR=${HOME_DIR}/kokkos-tools
elif [ $MACHINE = s1024454 ]; then
    export KK_TOOLS_DIR=${HOME_DIR}/kokkos-tools
elif [ $MACHINE = clamps ]; then
    export HOME_DIR="${HOME}/mjschmidt"
    export KK_TOOLS_DIR=${HOME_DIR}/kokkos-tools./bin/parPT /data/particleParams.yaml -v 2> data/a.err
    fi
fi

# simple kernel timer location
# export simple_kt=true
# export KOKKOS_PROFILE_LIBRARY=${KK_TOOLS_DIR}/kp_kernel_timer.so
# export PATH=${PATH}:${KK_TOOLS_DIR}

# space time stack
# export st_stack=true
# export KOKKOS_PROFILE_LIBRARY=${HOME_DIR}/kokkos-tools/profiling/space-time-stack/kp_space_time_stack.so
# export PATH=${PATH}:${HOME_DIR}/kokkos-tools/profiling/

# memory events
# export KOKKOS_PROFILE_LIBRARY=${HOME_DIR}/kokkos-tools/profiling/memory-events/kp_memory_events.so
# export PATH=${PATH}:${HOME_DIR}/kokkos-tools/profiling/

# kernel logger
# export KOKKOS_PROFILE_LIBRARY=${KK_TOOLS_DIR}/kp_kernel_logger.so
# export PATH=${PATH}:${KK_TOOLS_DIR}

export MACHINE=`hostname`
if [ $MACHINE = s1046231 ]; then
    if [ "$st_stack" = true ]
    then
        ./bin/parPT /data/particleParams.yaml -v > prof_results.txt
        subl prof_results.txt
        # echo "a"
    elif [ "$simple_kt" = true ]
    then
        ./bin/parPT /data/particleParams.yaml -v
        kp_reader *.dat > prof_results.txt
        rm s1046231*.dat
        subl prof_results.txt
        # echo "b"
        # rm prof_results.txt
    else
        if [ "$plot" = true ]
        then
            # run the program and redirect the error output (cpu)
            ./bin/parPT /data/particleParams.yaml --kokkos-threads=8 -v 2> data/a.err
            # ./bin/parPT /data/particleParams.yaml -v
            cd plotting
            python3 plotParticles.py3
            cd ..
        else
            # run the program and redirect the error output (cpu)
            ./bin/parPT /data/particleParams.yaml --kokkos-threads=8 -v 2> data/a.err
            # ./bin/parPT /data/particleParams.yaml -v
            # run the program and redirect screen and error output
            # ./parPT.exe > a.out 2> a.err
        fi
    fi
elif [ $MACHINE = s1024454 ]; then
    if [ "$simple_kt" = true ]
    then
        ./bin/parPT /data/particleParams.yaml -v
        kp_reader *.dat > prof_results.txt
        rm ${MACHINE}*.dat
        vim prof_results.txt
    else
        ./bin/parPT /data/particleParams.yaml -v 2> data/a.err
    fi
elif [ $MACHINE = clamps ]; then
    if [ "$simple_kt" = true ]
    then
        ./bin/parPT /data/particleParams.yaml -v
         kp_reader *.dat > prof_results.txt
         rm ${MACHINE}*.dat
        vim prof_results.txt
    else
        ./bin/parPT /data/particleParams.yaml -v 2> data/a.err
    fi
fi
