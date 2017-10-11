#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}

# Create directories for out-of-source build.
mkdir -p build/$BUILD_TYPE || true

# Execute cmake.
cd build/$BUILD_TYPE && cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../..

