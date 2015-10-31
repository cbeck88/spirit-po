#!/bin/bash
set -e
make clean
make "$@"
gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./main
