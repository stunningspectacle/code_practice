#!/bin/bash
adb -s $1 wait-for-devices
adb -s $1 root
sleep 5
#adb -s $1 remount
#adb -s $1 shell "rm -rf /system/app/RecursiveFunctionApp.apk"
#adb -s $1 shell "rm -rf /system/app/InfiniteThreadApp.apk"
 
i=0
while [ 1 ]
do
date
adb -s $1 wait-for-devices
adb -s $1 root
sleep 5
adb -s $1 shell "echo 8 > /proc/sys/kernel/printk"
adb -s $1 shell "echo Y > /sys/module/kernel/parameters/initcall_debug"
adb -s $1 shell "echo N > /sys/module/printk/parameters/console_suspend"
#adb -s $1 shell "monkey --throttle 200 -v --ignore-crashes --ignore-timeouts --ignore-security-exceptions --ignore-native-crashes --kill-process-after-error 3000000 >/dev/null"
#sleep 30
adb -s $1 shell "monkey --ignore-crashes --ignore-timeouts --kill-process-after-error --ignore-security-exceptions --throttle 500 -v 20000000"
i=$(($i+1))
echo "count $i"
#monkeys=`ps|egrep "crash|debuggerd|log|recursive|test|camera" |awk {'print $2'}`
#echo $monkeys
#       for p in $monkeys
#       do
#       echo "kill monkey process:$p"
#       kill -9 $p
#       done
#ps|grep crashreport|awk {'print $2'}|xargs kill -9
#rm -rf /logs/*
done
echo "======= exit========="

