#!/bin/bash
###### Usage: ./stability_warmboot.sh $deviceID
device=$1
LOOP_WB=1000
LOG=stability_warmboot_${device}.log
cat /dev/null > $LOG
i=1
while [ $i -le $LOOP_WB ]
do
	echo "WarmBoot LOOP $i started at BEHCH time: `date` DUT time: `adb -s $device shell date`" >> $LOG
	adb -s $device reboot
	sleep 10
	adb -s $device wait-for-device
	echo "WarmBoot LOOP $i ended at BEHCH time: `date` DUT time: `adb -s $device shell date`"  >> $LOG
	sleep 35
	adb -s $device root
	sleep 2
	adb -s $device shell getprop | grep boot >> $LOG
#	adb -s $device shell cat /sys/class/power_supply/intel_fuel_gauge/* >> $LOG
#	adb -s $device shell dmesg |grep smsc >> $LOG
#	adb -s $device shell dmesg |grep bq24192 >> $LOG
	let i=i+1
done
