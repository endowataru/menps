#!/bin/bash

export UCX_HANDLE_ERRORS=
export MYTH_BIND_WORKERS=0

for n_procs in `seq 3`; do
    for n_wks in `seq 3`; do
        for n_ths in `seq 3`; do
            export MYTH_NUM_WORKERS=$n_wks
            export CMPTH_NUM_WORKERS=$n_wks
            export MEDSM2_NUM_THREADS=$n_ths
            eval echo Testing MYTH_NUM_WORKERS=$n_wks \
                CMPTH_NUM_WORKERS=$n_wks \
                MEDSM2_NUM_THREADS=$MEDSM2_NUM_THREADS \
                ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} $n_procs \
                ${MPIEXEC_PREFLAGS} "${@:1}" ${MPIEXEC_POSTFLAGS}
            eval ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} $n_procs \
                ${MPIEXEC_PREFLAGS} "${@:1}" ${MPIEXEC_POSTFLAGS}
        done
    done
done

