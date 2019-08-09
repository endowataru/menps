#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}
if [ -z "${BUILD_DIR}" ]; then BUILD_DIR=./build/$BUILD_TYPE; fi
if [ -z "${SOURCE_DIR}" ]; then SOURCE_DIR=$(cd $(dirname $0); pwd); fi

# Create directories for out-of-source build.
mkdir -p $BUILD_DIR || true
cd $BUILD_DIR

# Detect the environment.
C_COMPILER=mpicc
if command -v mpicxx > /dev/null; then
    CXX_COMPILER=mpicxx
else
    CXX_COMPILER=mpic++
fi

# Execute cmake.
cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$C_COMPILER \
    -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
    "${@:2}" \
    ${SOURCE_DIR}

echo "$@" >> cmake.bash.log

