#!/bin/bash

set -e
set -x

rm -rf build
mkdir build
cd build

cmake "$@" ..
make VERBOSE=1
gdb -return-child-result -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./test_libintl
