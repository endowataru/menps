#!/bin/sh

set -eu

build_type=$1
count=$2

./build/${build_type}/mgbase/mgbase-unittest --gtest_repeat=$count --gtest_break_on_failure

for i in `seq 1 4`; do
    MGCOM_DEVICE=mpi1 mpirun -n $i ./build/${build_type}/mgcom/mgcom-unittest --gtest_repeat=$count --gtest_break_on_failure
done

for i in `seq 1 4`; do
    MGCOM_DEVICE=mpi1 mpirun -n $i ./build/${build_type}/mgas2/mgas2-unittest --gtest_repeat=$count --gtest_break_on_failure
done

