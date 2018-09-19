#!/bin/bash

start_time=$(date +%F-%H-%M-%S)
TRACEFILE="trace."$start_time".log"
KERNELFILE="kernel."$start_time".log"
MIN_INT=5

keyword="rockchip"
lastdump=0

getTrace() {
    dumptime=$(date +%F-%H-%M-%S)
    echo "---------------------$dumptime------------------------" >> $TRACEFILE
    adb shell "cat /d/tracing/trace" >> $TRACEFILE
    adb shell "echo 0 > /d/tracing/trace"
}

while true;
do
#    tail -f $1 | while read line;
    adb wait-for-device && adb root && adb wait-for-device
    loop_start=$(date +%F-%H-%M-%S)
    echo "---------------------$loop_start------------------------" >> $KERNELFILE
    adb shell cat /proc/kmsg | while read line;
    do
        echo $line >> $KERNELFILE
        if echo $line | grep -q -i $keyword; then
            now=$(date +%s)
            interval=$(($now - $lastdump))
            if [ $interval -gt $MIN_INT ]; then
                echo "interval=$interval, lastdump=$lastdump dump"
                getTrace
                echo $line
                lastdump=$now
            else
                echo "interval=$interval, lastdump=$lastdump not dump"
            fi
        fi
    done 
done
