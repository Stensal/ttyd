#!/bin/bash

if [ -d /ishell/workspace/wandbox/t-gdbtest ]; then
    rm -rf /ishell/workspace/wandbox/t-gdbtest
fi
cp -a /t-gdbtest /ishell/workspace/wandbox
cd /ishell/workspace/wandbox/t-gdbtest
./build.sh
# redirect stdout/stderr to files
exec >>  /var/log/ishell-stdout.log
exec 2>> /var/log/ishell-stderr.log

exec /bin/ishell $@
