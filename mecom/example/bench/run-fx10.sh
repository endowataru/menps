#!/bin/bash

echo "
module unload TCSuite && module load GCC ;
for s in \`seq 3 22\`; do
    for j in \`seq 32\`; do
        mpiexec -n $2 $1 -o ${1##*/}.p=$2.t=$3.yaml -t $3 -d 5 -m 0 -s \$((2**s))
    done
done
" |
pjsub -L "rscgrp=short" -L "node=$2" -L "elapse=2:00:00"

