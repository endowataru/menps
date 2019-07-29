#!/bin/bash

docker run -it --mount type=bind,src=$(pwd),dst=/menps \
    --security-opt seccomp=unconfined \
    wendo/mpich

