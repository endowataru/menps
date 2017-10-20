#!/bin/bash

echo '
cd $PBS_O_WORKDIR
export MECOM_IBV_DIRECT=0
export MECOM_IBV_DEVICE=mlx5_0
for j in `seq 16`; do
    for s in `seq 3 22`; do
        MECOM_IBV_NUM_QPS_PER_PROC=1 MYTH_NUM_WORKERS=10 mpirun -n '$2' '$1' -o '${1##*/}'.nqps=1.p='$2'.t='$3'.yaml -t '$3' -d 5 -m 0 -s $((2**$s))
    done
done
' |
qsub -q regular -l select=$2:ncpus=20 -l walltime=180:00

