#!/bin/bash

BUILD_DIR=/build ./cmake.bash Debug -D MEDSM2_ULT_ITF=SCT
BUILD_DIR=/build ./make.bash Debug VERBOSE=1 -j

