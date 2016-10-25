#!/bin/sh

mkdir -p build &&
cd build &&
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=../dist .. \
	-DCMAKE_VERBOSE_MAKEFILE=ON &&
make mf_test &&
lldb-3.8 -o "break set -E c++" -- ./mf_test -s $1
