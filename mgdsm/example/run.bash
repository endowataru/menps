#!/bin/bash

echo '
cd $PBS_O_WORKDIR
mpirun -n '$2' '$1' > out.yaml
' |
qsub -q u-debug -l select=$2 -l walltime=0:30:00 -W group_list=gc64

