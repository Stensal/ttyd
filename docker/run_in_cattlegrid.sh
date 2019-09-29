#!/bin/bash
mypath=$(dirname $(readlink -f $0))

cattlecell=$1
shift

AS=$((128*1024*1024))
CPU=$((6*60*60))
DATA=$((128*1024*1024))

chown 1000:1000 -R /ishell/workspace/wandbox/$cattlecell/store
/usr/bin/env \
    HOME=/home/jail \
    COMPILER_DISPLAY_NAME=stensal \
    COMPILER_CHECK_NAME=stensal \
    KLARAM_REMOVE_SRCLOC_PREFIX=/home/jail \
    DTS_CHECK_SWITCH=0x00000f13 \
    DTS_MEMORY_UNINIT_CHECK=warning \
    DTS_COLORING_MSG=1 \
    DTS_STUDENT_MODE=1 \
    /bin/prlimit \
    --core=0 --as=${AS} --cpu=${CPU} --data=${DATA} --fsize=${DATA} --nofile=16 --nproc=64 -- \
    /bin/cattlegrid --rootdir=./jail \
    --mount=/ishell_bin,/lib \
    --rwmount=/tmp=/tmp,/home/jail=/ishell/workspace/wandbox/$cattlecell/store \
    --chdir=/home/jail --uid=1000 /ishell_bin/run_prog.sh $@
