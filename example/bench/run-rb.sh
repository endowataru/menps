#!/bin/bash

echo '
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR
for j in `seq 4`; do
    for t in `seq 10`; do
        MGCOM_IBV_NUM_QPS_PER_PROC='$3' mpirun -n '$2' '$1' -o '${1##*/}'.nqps='$3'.p='$2'.t=$t.yaml -t $t -d 5 -m 0
    done
done
' |
qsub -q u-short -l select=$2 -l walltime=0:30:00 -W group_list=gc64

