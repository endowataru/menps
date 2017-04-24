#!/bin/bash

echo '
cd $PBS_O_WORKDIR
export MGCOM_IBV_DIRECT=0
export MGCOM_IBV_DEVICE=mlx5_0
for j in `seq 16`; do
    for t in `seq 16`; do
        MGCOM_IBV_NUM_QPS_PER_PROC='$3' MYTH_NUM_WORKERS=19 mpirun -n '$2' '$1' -o '${1##*/}'.nqps='$3'.p='$2'.t=$t.yaml -t $t -d 5 -m 0
    done
done
' |
qsub -q regular -l select=$2:ncpus=20 -l walltime=180:00

