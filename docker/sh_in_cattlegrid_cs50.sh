#!/bin/bash
mypath=$(dirname $(readlink -f $0))

cattlecell=$1
shift

MEM=$((1024*1024*1024))
AS=$MEM
CPU=$((1*60*60))
DATA=$MEM
XUID=2000
XGID=20000
NFILES=256

CELL=/ishell/workspace/sbox/$cattlecell
STORE=${CELL}/store
JAIL=${CELL}/jail

if [ -d ${STORE} ]; then
    cd ${CELL};
    cp /xshell/bin/greeting.sh   ${STORE}/.bashrc
    cp /xshell/bin/greeting.sh   ${STORE}/.profile
    # make all folders accessible
    find ${CELL} -type d -exec chmod 0777 {} \;

    /usr/bin/env \
	PS1='$ ' \
	HOME=/home/user \
	PATH=.:/bin:/xshell/bin:/usr/bin \
	COMPILER_DISPLAY_NAME=stensal \
	COMPILER_CHECK_NAME=stensal \
	KLARAM_REMOVE_SRCLOC_PREFIX=/home/user \
	DTS_CHECK_SWITCH=0x00000f13 \
	DTS_MEMORY_UNINIT_CHECK=warning \
	DTS_COLORING_MSG=1 \
	DTS_GRACE_ABORT=0  \
	DTS_STUDENT_MODE=1 \
	DTS_REPORT_UNRELEASED_MEMORY=1 \
	/bin/nice \
	/sbin/prlimit \
	--core=0 --as=${AS} --cpu=${CPU} --data=${DATA} --fsize=${DATA} --nofile=${NFILES} --nproc=64 -- \
	/sbin/cattlegrid --rootdir=./ebox \
	--mount=/etc,/bin,/xshell,/lib,/usr/lib,/sjacket/lib,/sjacket/usr/lib,/sjacket/etc,/usr/share/terminfo \
	--rwmount=/tmp=/tmp,/home/user=./store \
	--uid=${XUID} --gid=${XGID} \
	--chdir=/home/user \
	/xshell/bin/mybash.sh
else
	#--mount=/etc,/bin,/xshell,/lib,/usr/lib,/sjacket/lib,/sjacket/usr/lib,/sjacket/etc,/usr/share/terminfo \
    # possible attack
    exit 1;
fi
