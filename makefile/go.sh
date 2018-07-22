#!/bin/bash
make
#if [ "$1" == "" ]; then
#    echo "no arg"
#    exit
#fi
! [ -f $1 ] && {
    echo "$1: no such file"
    exit
}

[ $# -lt 1 ] && exit

./$1

