#!/bin/sh
make build_test &&
./dist/mf_test -s $1
#./dist/mf_test $1 2>&1 | c++filt
