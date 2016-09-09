#!/bin/bash

echo "
cd $PBS_O_WORKDIR
export MGCOM_IBV_DIRECT=0
export MGCOM_IBV_DEVICE=mlx5_0
for t in 1 2 4 8 12 16; do
    for s in \`seq 3 22\`; do
        for j in \`seq 16\`; do
            mpirun -n $2 $1 -o ${1##*/}.p=$2.t=\$t.offload.yaml -t \$t -d 5 -m 0 -s \$((2**s))
        done
    done
done

export MGCOM_IBV_DIRECT=1
export MGCOM_IBV_DEVICE=mlx5_0
for t in 1 2 4 8 12 16; do
    for s in \`seq 3 22\`; do
        for j in \`seq 16\`; do
            mpirun -n $2 $1 -o ${1##*/}.p=$2.t=\$t.direct.yaml -t \$t -d 5 -m 0 -s \$((2**s))
        done
    done
done
" |
qsub -q regular -l select=$2:ncpus=20 -l walltime=1200:00

