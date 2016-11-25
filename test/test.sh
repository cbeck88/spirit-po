#!/bin/bash
set -e
rm -rf stage
b2
gdb -return-child-result -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./stage/main
