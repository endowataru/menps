#!/bin/bash

echo "
cd $PBS_O_WORKDIR
export MECOM_IBV_DIRECT=0
export MECOM_IBV_DEVICE=mlx5_0
export MECOM_IBV_MAX_NUM_OFFLOAD_THREADS=$3
#for s in \`seq 3 22\`; do
for t in \`seq 1 8\`; do
    for j in \`seq 32\`; do
        mpirun -n $2 $1 -o ${1##*/}.p=$2.e=$3.yaml -t \$t -d 5 -s 64
    done
done
        # -m 0
" |
qsub -q regular -l select=$2:ncpus=20 -l walltime=180:00

