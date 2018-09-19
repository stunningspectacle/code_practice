#!/bin/bash
# leo.he@intel.com

adb=

if [ $# -gt 0 ]; then
    adb="adb -s $1"
else
    adb="adb"
fi

$adb root
sleep 3
$adb shell "rm -rf /logs/*"

while true
do
   # TODO: -s
echo "waiting for ADB ..."
$adb wait-for-device
timeout=$(($RANDOM%$2))
count=$((count+1))
echo "count $count, reboot timeout $timeout"
sleep 3
$adb root
sleep 3
$adb shell "echo 8 > /proc/sys/kernel/printk"
$adb shell "echo Y > /sys/module/kernel/parameters/initcall_debug"
$adb shell "echo N > /sys/module/printk/parameters/console_suspend"
sleep $timeout
#     if [[ ! -z `adb shell cat /logs/history_event | grep FABRICERR` ]]; then
#       die "Fabric occur, exit!";
#     elif [[ ! -z `adb shell cat /logs/history_event | grep IPANIC` ]]; then
#       die "Panic occur, exit!"
#     fi
$adb shell reboot android
done
