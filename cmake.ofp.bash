#!/bin/bash
export C_COMPILER=mpiicc
export CXX_COMPILER=mpiicpc
if [ -z "${SOURCE_DIR}" ]; then SOURCE_DIR=$(cd $(dirname $0); pwd); fi
${SOURCE_DIR}/cmake.bash $@
