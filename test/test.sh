#!/bin/bash
set -e
export CXXFLAGS="$CXXFLAGS"
echo "$CXXFLAGS"
make clean
make
./main
