#!/bin/bash -x
echo "$@"
echo "change directory to $1"
cd $1
pwd
/usr/bin/gdb --interpreter=mi $2
