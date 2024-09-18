#!/bin/sh
prefix="/usr"
subdir="$prefix/lib/sublage"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$subdir/stdlib
$subdir/bin/sublagec $*
