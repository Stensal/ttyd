#!/bin/bash

# redirect stdout/stderr to files
exec >>  /var/log/ishell-stdout.log
exec 2>> /var/log/ishell-stderr.log

exec /sbin/ishell $@
