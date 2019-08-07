#!/bin/bash
mypath=$(dirname $(readlink -f $0))

cattlecell=$1
shift


/usr/bin/env \
    HOME=/home/jail \
    /bin/prlimit \
    --core=0 --as=1073741824  --cpu=30 --data=134217728 --fsize=134217728 --nofile=256 --nproc=512 -- \
    /bin/cattlegrid --rootdir=./jail \
    --mount=/ishell_bin,/lib \
    --rwmount=/tmp=/tmp,/home/jail=/ishell/workspace/wandbox/$cattlecell/store \
    --devices=/dev/null,/dev/zero,/dev/full,/dev/random,/dev/urandom \
    --chdir=/home/jail /ishell_bin/run_prog.sh $@
