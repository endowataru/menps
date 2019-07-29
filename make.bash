#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}
if [ -z "${BUILD_DIR}" ]; then BUILD_DIR=./build/$BUILD_TYPE; fi

cd ${BUILD_DIR}

cmake --build . -- "${@:2}"

