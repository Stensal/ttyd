#!/bin/bash

GDBTEST=/ishell/workspace/sbox/t-gdbtest

if [ -d ${GDBTEST} ]; then
    chmod 770 ${GDBTEST}
    rm -rf ${GDBTEST}
fi
cp -a /t-gdbtest /ishell/workspace/sbox
cd ${GDBTEST}/store
./build.sh
find ${GDBTEST} -type d -exec chmod 0770 {} \;
find ${GDBTEST} -type d -exec chown -R tester:0 {} \;
# redirect stdout/stderr to files
exec >>  /var/log/ishell-stdout.log
exec 2>> /var/log/ishell-stderr.log

exec /bin/ishell $@
