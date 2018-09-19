#!/bin/bash

adb=

if [ $# -gt 0 ]; then
    adb="adb -s $1"
else
    adb="adb"
fi

$adb wait-for-device
$adb shell "echo 1 > /d/mali_platform/dvfs_off"
echo "devfs_off:"
$adb shell "cat /d/mali_platform/dvfs_off"

function echo_pm_level() {
    $adb shell "echo $1 > /d/mali_platform/pm_level"
    sleep 1
    pm_level=$($adb shell "cat /d/mali_platform/pm_level")
    echo "pm_level:" $pm_level
}

prev_pm_level=0
pm_level=0

while true; do
    while true; do
        pm_level=$(($RANDOM % 4 + 1))
        if [ "$prev_pm_level" != "$pm_level" ]; then
            break
        fi
    done
    prev_pm_level=$pm_level

    echo_pm_level $pm_level
done
