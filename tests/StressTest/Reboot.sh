#!/bin/bash
# leo.he@intel.com

adb -s $1 root
sleep 3
adb -s $1 shell "rm -rf /logs/*"

while true
do
   # TODO: -s
echo "waiting for ADB ..."
adb -s $1 wait-for-device
timeout=$((RANDOM%$2))
count=$((count+1))
echo "count $count, reboot timeout $timeout"
sleep 3
adb -s $1 root
sleep 3
adb -s $1 shell "echo 8 > /proc/sys/kernel/printk"
adb -s $1 shell "echo Y > /sys/module/kernel/parameters/initcall_debug"
adb -s $1 shell "echo N > /sys/module/printk/parameters/console_suspend"
sleep $timeout
#     if [[ ! -z `adb shell cat /logs/history_event | grep FABRICERR` ]]; then
#       die "Fabric occur, exit!";
#     elif [[ ! -z `adb shell cat /logs/history_event | grep IPANIC` ]]; then
#       die "Panic occur, exit!"
#     fi
adb -s $1 shell reboot android
done
