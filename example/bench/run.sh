#!/bin/bash

echo "module unload TCSuite && module load GCC ;
      for j in \`seq 32\`; do mpiexec -n $2 $1 -o output.p=$2.t=$3.yaml -t $3 -d 5; done" |
pjsub -L "rscgrp=debug" -L "node=$2" -L "elapse=30:00"

