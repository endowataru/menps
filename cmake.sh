#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}

# Create directories for out-of-source build.
mkdir -p build/$BUILD_TYPE || true

# Detect the environment.
C_COMPILER=mpicc
if command -v mpicxx > /dev/null; then
    CXX_COMPILER=mpicxx
else
    CXX_COMPILER=mpic++
fi

# Execute cmake.
cd build/$BUILD_TYPE && \
cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$C_COMPILER \
    -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
    "${@:2}" \
    ../..

