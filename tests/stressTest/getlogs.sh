#!/bin/bash

logname=$(date +%F-%H-%M-%S).log
adb=

if [ $# -gt 0 ]; then
    adb="adb -s $1"
else
    adb="adb"
fi

while true;
do
    $adb wait-for-device
    $adb root
    $adb wait-for-device
    now=$(date +%F-%H-%M-%S)
    echo "-----------------------$now--------------------------" >> $logname
    $adb shell dmesg -c >> $logname
    $adb shell "logcat -f /dev/kmsg &"
    $adb shell cat /proc/kmsg >> $logname
done


