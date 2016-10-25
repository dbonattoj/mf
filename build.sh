#!/bin/bash

mkdir -p build &&
cd build &&
cmake \
    -DCMAKE_BUILD_TYPE=$1 \
    -DCMAKE_INSTALL_PREFIX=../dist .. &&
make mf &&
make install
