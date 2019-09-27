#!/bin/bash
mypath=$(dirname $(readlink -f $0))

cattlecell=$1
shift

AS=$((128*1024*1024))
CPU=$((6*60*60))
DATA=$((128*1024*1024))

/usr/bin/env \
    HOME=/home/jail \
    /bin/prlimit \
    --core=0 --as=${AS} --cpu=${CPU} --data=${DATA} --fsize=${DATA} --nofile=16 --nproc=64 -- \
    /bin/cattlegrid --rootdir=./jail \
    --mount=/ishell_bin,/lib \
    --rwmount=/tmp=/tmp,/home/jail=/ishell/workspace/wandbox/$cattlecell/store \
    --devices=/dev/null,/dev/zero,/dev/full,/dev/random,/dev/urandom \
    --chdir=/home/jail /ishell_bin/run_prog.sh $@
