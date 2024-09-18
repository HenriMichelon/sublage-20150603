#!/bin/sh
if [ "`id -u`" -ne 0 ]
then
    echo 'Error: `install.sh` must be executed by root or using `sudo`.'
    exit
fi
prefix="/usr"
cp -v lib/SUBLAGERT $prefix/lib
cp -v bin/sublage bin/sublagec $prefix/bin
chmod -v +rx $prefix/bin/sublage $prefix/bin/sublagec
mkdir -pv $prefix/lib/sublage/stdlib
cp -v lib/sublage/stdlib/* $prefix/lib/sublage/stdlib
mkdir -pv $prefix/lib/sublage/bin
cp -v lib/sublage/bin/* $prefix/lib/sublage/bin
chmod -v +rx $prefix/lib/sublage/bin/*
