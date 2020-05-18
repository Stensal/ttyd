#!/bin/bash
mypath=$(dirname $(readlink -f $0))

pkg=ishell-cs50

if [ -f $pkg.tar ]; then
    rm $pkg.tar
    rm -f $pkg.tar.xz
fi

docker save -o $pkg \
       dockerhub.stensalinc.com/i386/$pkg:latest

xz -z $pkg.tar

if [ -f $pkg.tar.xz ]; then
    cmd='rm -f $pkg.tar.xz'
    ssh cadmin@45.56.80.57 $cmd
    scp $pkg.tar.xz cadmin@45.56.80.57:~/
    if [ "$?" == "0" ]; then
        rm $pkg.tar.xz
        cmd='xz -dc $pkg.tar.xz | docker load'
        ssh cadmin@45.56.80.57 $cmd
    fi
fi
