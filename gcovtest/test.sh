#!/bin/bash

CPU_NUMS=2
MAKE_JOBS=0

for i in $(cat /proc/cpuinfo | grep processor | cut -d: -f2);
do
    ((MAKE_JOBS++))
done
((MAKE_JOBS*=2))
if [ $MAKE_JOBS -lt 2 ]; then
    MAKE_JOBS=2
fi

echo MAKE_JOBS=$MAKE_JOBS
