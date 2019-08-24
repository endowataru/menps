#!/bin/bash

set -e

for i in `seq 8`; do
    # set the variable (whose name is $1) to $i
    eval export $1=$i
    eval echo Testing $1='$'$1 "${@:2}"
    eval "${@:2}"
done

