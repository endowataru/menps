#!/bin/bash

echo "
cd $PBS_O_WORKDIR
export MGCOM_IBV_DIRECT=0
export MGCOM_IBV_DEVICE=mlx5_0
for s in \`seq 3 22\`; do
    for j in \`seq 32\`; do
        mpirun -n $2 $1 -o ${1##*/}.p=$2.t=$3.yaml -t $3 -d 5 -m 0 -s \$((2**s))
    done
done
" |
qsub -q regular -l select=$2:ncpus=20 -l walltime=180:00

