#!/bin/sh
make build_test &&
lldb-3.8 -o "break set -E c++" -- dist/mf_test -s $1
