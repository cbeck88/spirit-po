#!/bin/bash
set -e
rm -rf stage

if hash b2 2>/dev/null; then
  b2 "$@"
elif hash bjam 2>/dev/null; then
  bjam "$@"
else
  echo >&2 "Require b2 or bjam but it was not found. Aborting."
  exit 1
fi

gdb -return-child-result -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args ./stage/main
