#!/bin/bash

echo "module unload TCSuite && module load GCC ;
      for j in \`seq 10\`; do mpiexec -n $2 $1; done" |
pjsub -L "rscgrp=debug" -L "node=$2" -L "elapse=10:00"

