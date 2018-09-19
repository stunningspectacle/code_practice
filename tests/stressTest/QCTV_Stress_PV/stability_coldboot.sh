#!/bin/bash
LOOP=1000
SERIAL=$1
sudo chmod 666 $SERIAL
sudo stty -F $SERIAL 19200
date=`date +%Y%m%d%H%M%S`
LOG=stability_coldboot_${date}.log
cat /dev/null > $LOG
i=1
while [ $i -le $LOOP ]
do
	echo -e "\x5c\xff" > $SERIAL;sleep 10; echo -e "\x5c\x00" > $SERIAL
	echo "LOOP $i POWER OFF at: `date`" >> $LOG
	sleep 5
	echo -e "\x5c\xff" > $SERIAL;sleep 4; echo -e "\x5c\x00" > $SERIAL
	echo "LOOP $i POWER ON at: `date`" >> $LOG
	sleep 60
	let i=i+1
done
	
