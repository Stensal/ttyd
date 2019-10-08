#!/bin/bash
mypath=$(dirname $(readlink -f $0))

cattlecell=$1
shift

MEM=$((1024*1024*1024))
AS=$MEM
CPU=$((1*60*60))
DATA=$MEM
XUID=2000

cp  /ishell_bin/greeting.sh   /ishell/workspace/wandbox/$cattlecell/store/.profile
find /ishell/workspace/wandbox/$cattlecell -type d -exec chmod 0770 {} \;
find /ishell/workspace/wandbox/$cattlecell -type d -exec chown ${XUID}:0 -R {} \;

/usr/bin/env \
    PS1='$ ' \
    HOME=/home/user \
    PATH=.:/ishell_bin \
    COMPILER_DISPLAY_NAME=stensal \
    COMPILER_CHECK_NAME=stensal \
    KLARAM_REMOVE_SRCLOC_PREFIX=/home/user \
    DTS_CHECK_SWITCH=0x00000f13 \
    DTS_MEMORY_UNINIT_CHECK=warning \
    DTS_COLORING_MSG=1 \
    DTS_STUDENT_MODE=1 \
    /bin/nice \
    /bin/prlimit \
    --core=0 --as=${AS} --cpu=${CPU} --data=${DATA} --fsize=${DATA} --nofile=16 --nproc=64 -- \
    /bin/cattlegrid --rootdir=./user \
    --mount=/ishell_bin,/lib,/usr/lib,/sjacket/lib,/sjacket/usr/lib,/sjacket/etc \
    --rwmount=/tmp=/tmp,/home/user=/ishell/workspace/wandbox/$cattlecell/store \
    --chdir=/home/user --uid=${XUID} /ishell_bin/sh +m +l
