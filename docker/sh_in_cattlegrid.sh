#!/bin/bash
mypath=$(dirname $(readlink -f $0))

cattlecell=$1
shift

/usr/bin/env \
    HOME=/home/jail \
    PATH=.:/ishell_bin \
    /bin/prlimit \
    --core=0 --as=10737418240 --cpu=43200 --data=134217728 --fsize=134217728 --nofile=256 --nproc=512 -- \
    /bin/cattlegrid --rootdir=./jail \
    --mount=/ishell_bin,/lib,/usr/lib,/usr/local,/usr/share \
    --rwmount=/tmp=/tmp,/home/jail=/ishell/workspace/wandbox/$cattlecell/store \
    --devices=/dev/null,/dev/zero,/dev/full,/dev/random,/dev/urandom,/dev/tty \
    --chdir=/home/jail /ishell_bin/sh
