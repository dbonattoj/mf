#!/bin/sh
make dist/mf_test &&
lldb-3.6 -O "break set -E c++" dist/mf_test $1
