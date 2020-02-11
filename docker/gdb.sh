#!/bin/bash

cattlecell=$1
shift

CELL=/ishell/workspace/wandbox/$cattlecell
STORE=${CELL}/store

cd ${STORE}
/usr/bin/gdb $@
