#!/bin/sh
tail -f "/home/timlenertz/Desktop/programming/mf/debug_$1.txt" | c++filt
