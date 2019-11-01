#!/bin/bash
export C_COMPILER=mpiicc
export CXX_COMPILER=mpiicpc
${SOURCE_DIR}/cmake.bash $@
