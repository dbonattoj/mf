#!/bin/sh
make build_test &&
lldb-3.8 -O "break set -E c++" -- dist/mf_test -s $1
