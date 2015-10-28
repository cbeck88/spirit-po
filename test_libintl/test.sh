#!/bin/bash

set -e

rm -rf build
mkdir build
cd build

cmake -DCMAKE_CXX_FLAGS="-O3 -Wall -Werror $CXXFLAGS" ..
#cmake -DCMAKE_CXX_FLAGS="-O0 -g3 -Wall -Werror" ..
make
gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./test_libintl
