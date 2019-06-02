#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}

cd build/$BUILD_TYPE && ctest "${@:2}"

