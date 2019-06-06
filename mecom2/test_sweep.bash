#!/bin/bash

export UCX_HANDLE_ERRORS=
export MYTH_BIND_WORKERS=0

MPIRUN_COMMAND="mpirun --allow-run-as-root"

for n_procs in `seq 3`; do
    for n_wks in `seq 3`; do
        #for n_ths in `seq 3`; do
            export MYTH_NUM_WORKERS=$n_wks
            export CMPTH_NUM_WORKERS=$n_wks
            #export MEDSM2_NUM_THREADS=$n_ths
            eval echo Testing MYTH_NUM_WORKERS=$n_wks \
                CMPTH_NUM_WORKERS=$n_wks \
                ${MPIRUN_COMMAND} -n $n_procs "${@:1}"
            eval ${MPIRUN_COMMAND} -n $n_procs "${@:1}"
        #done
    done
done

