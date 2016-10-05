#!/bin/sh
make build_test &&
while ./dist/mf_test $1; do :; done

