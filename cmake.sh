#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}

# Create directories for out-of-source build.
mkdir -p build/$BUILD_TYPE || true

# Detect the environment.

if [[ $HOSTNAME =~ "oakleaf" ]]; then
    # Check whether GCC is being used or not.
    if command -v mpig++px > /dev/null; then
        # GCC
        C_COMPILER=mpigccpx
        CXX_COMPILER=mpig++px
    else
        # Fujitsu compiler
        C_COMPILER=mpifccpx
        CXX_COMPILER=mpiFCCpx
    fi
else
    C_COMPILER=mpicc
    if command -v mpicxx > /dev/null; then
        CXX_COMPILER=mpicxx
    else
        CXX_COMPILER=mpic++
    fi
fi

# Execute cmake.
cd build/$BUILD_TYPE && \
cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$C_COMPILER \
    -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
    "${@:2}" \
    ../..

