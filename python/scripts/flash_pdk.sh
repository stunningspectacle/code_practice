#!/bin/bash
adb reboot bootloader
fastboot erase cache
fastboot erase data

if [ ! -f boot.img ]; then
    echo "No boot.img"
    exit
fi
fastboot flash boot boot.img

if [ ! -f system.img ]; then
    echo "No system.img"
    exit
fi
fastboot flash system system.img

fastboot reboot
