#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}
if [ -z "${BUILD_DIR}" ]; then BUILD_DIR=./build/$BUILD_TYPE; fi
if [ -z "${SOURCE_DIR}" ]; then SOURCE_DIR=$(cd $(dirname $0); pwd); fi

# Create directories for out-of-source build.
mkdir -p $BUILD_DIR || true
cd $BUILD_DIR

# Detect the environment.
if [ -z "${C_COMPILER}" ]; then
    if command -v icc > /dev/null; then
        C_COMPILER=icc
    else
        C_COMPILER=cc
    fi
fi
if [ -z "${CXX_COMPILER}" ]; then
    if command -v icpc > /dev/null; then
        CXX_COMPILER=icpc
    else
        CXX_COMPILER=c++
    fi
fi

# Execute cmake.
cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$C_COMPILER \
    -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
    "${@:2}" \
    ${SOURCE_DIR}

echo "$@" >> cmake.bash.log

