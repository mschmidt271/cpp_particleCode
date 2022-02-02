#!/bin/bash

# OpenMP environment variables
export OMP_PROC_BIND=spread
export OMP_PLACES=cores

# whether we are on a remote computer and don't want to plot
# FIXME: change this to string matching so we only need one field
# export remote=true
# export laptop=false
export laptop=true
export remote=false
export run_cuda=false

# number of ensemble members
export totens=10

# location on Nick's workstation
# export KK_TOOLS_DIR=/home/pfsuser/mjschmidt/kokkos-tools\
# standard location
export KK_TOOLS_DIR=${HOME}/kokkos-tools

# simple kernel timer location
export simple_kt=true
export KOKKOS_PROFILE_LIBRARY=${KK_TOOLS_DIR}/kp_kernel_timer.so
export PATH=${PATH}:${KK_TOOLS_DIR}

# base of the filename that will be concatenated with number of cores
export fname="prof_results"
# make dirs for the different outputs
mkdir -p out
mkdir -p err
mkdir -p results

if [ "$laptop" = true ]
then
    export ncores=$(sysctl -n hw.ncpu)
    # do a few less so as not to overrun things
    let "ncores-=5"
    printf "Running profiler up to $ncores cores\n"
    for ((ens = 05; ens <= totens; ens++))
    do
        for (( ncore = 1; ncore <= $ncores; ncore++ ))
        do
            export OMP_NUM_THREADS=$ncore
            printf "Running for ${ncore} cores...\n"
            export f="results_${ens}/${fname}_${ncore}.txt"
            ../bin/parPT /data/particleParams.yaml -v > "out/${fname}_${ncore}.out"\
                2> "err/${fname}_${ncore}.err"
            kp_reader *.dat > $f
            # mike's laptop
            rm s1046231*.dat
        done
    done
elif [ "$remote" = true ]
then
    if [ "$run_cuda" = false ]
    then
        export ncores=$(grep -c ^processor /proc/cpuinfo)
        # do a few less so as not to overrun things
        let "ncores-=6"
        printf "Running profiler up to $ncores cores\n"
        for (( ncore = 1; ncore <= 2; ncore++ ))
        do
            export OMP_NUM_THREADS=$ncore
            printf "Running for ${ncore} cores...\n"
            export f="results/${fname}_${ncore}.txt"
            ../bin/parPT /data/particleParams.yaml -v > "out/${fname}_${ncore}.out"\
                2> "err/${fname}_${ncore}.err"
            kp_reader *.dat > $f
            # pete's workstation
            rm s1024454*.dat
            # nick's workstation
            # rm clamps-*.dat
        done
    elif [ "$run_cuda" = true ]
    then
        printf "Running profiler using cuda\n"
        export f="results/${fname}_cuda.txt"
        ../bin/parPT /data/particleParams.yaml -v > "out/${fname}_cuda.out"\
            2> "err/${fname}_cuda.err"
        kp_reader *.dat > $f
        # pete's workstation
        rm s1024454*.dat
    fi
fi
