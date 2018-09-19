#!/bin/bash
LOOP=2500
SERIAL=$1
sudo chmod 666 $SERIAL
sudo stty -F $SERIAL 19200
date=`date +%Y%m%d%H%M%S`
LOG=stability_sleepwakeup_${date}.log
cat /dev/null > $LOG
i=1
while [ $i -le $LOOP ]
do
        echo "Loop $i Wakeup @ `date`" >> $LOG
        echo -e "\x5c\xff" > $SERIAL;sleep 0.5; echo -e "\x5c\x00" > $SERIAL
        sleep 5
        echo "Loop $i Sleep @ `date`" >> $LOG
        echo -e "\x5c\xff" > $SERIAL;sleep 0.5; echo -e "\x5c\x00" > $SERIAL
        sleep 10
        let i=i+1
done
