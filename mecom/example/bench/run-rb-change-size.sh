#!/bin/bash

echo '
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR
#module unload intel/17.0.2.174
#module unload intel-mpi/2017.2.174
#module load mvapich2/2.2/gnu
for j in `seq 8`; do
    for s in `seq 3 22`; do
        MYTH_NUM_WORKERS='$3' MECOM_IBV_NUM_QPS_PER_PROC='$4' \
        mpirun -n '$2' '$1' -o '${1##*/}'.p='$2'.nwks='$3'.nqps='$4'.t='$5'.yaml \
        -t '$5' -d 5 -m 0 -s $((2**$s))
    done
done
' |
qsub -q u-debug -l select=$2 -l walltime=0:30:00 -W group_list=gc64
#qsub -q u-short -l select=$2 -l walltime=1:30:00 -W group_list=gc64

