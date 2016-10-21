#!/bin/sh

mkdir -p build &&
cd build &&
cmake -DCMAKE_INSTALL_PREFIX=../dist .. &&
make &&
make install
