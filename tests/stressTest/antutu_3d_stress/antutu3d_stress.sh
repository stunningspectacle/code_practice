#!/bin/bash
adb=
count=0
activity=com.antutu.ABenchMark/com.antutu.benchmark.test3d.Antutu3DLoader

if [ $# -gt 0 ]; then
    adb="adb -s $1"
else
    adb="adb"
fi

while true; do
    if ! $adb shell dumpsys activity | grep -q Native3DLoader; then
        count=$((count+1))
        echo "count =" $count
        $adb shell am start -n $activity
    fi
    sleep 1
done

