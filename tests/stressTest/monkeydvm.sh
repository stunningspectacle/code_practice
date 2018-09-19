#!/bin/bash

# First arg (if any) is board id
_id=$1
if [[ $_id == "" ]]; then
    _log_file="monkey.log"
else
    _log_file="monkey.$1.log"
fi

# Number of monkey inputs
_ninputs=20000000

log_date() {
    echo "["`date +%Y-%m-%d\ %H:%M:%S`"] $@"
}

# Make sure adb -s $_id runs as root
rm -f "$_log_file"
adb root
sleep 2
adb wait-for-devices

#RemoveInternalAPK
adb remount
sleep 2
if [ -f internalAPKList.txt ]; then
    for apk in `cat internalAPKList.txt`
            do
                    adb shell rm $apk
            done

    sleep 10
fi

adb shell "rm /system/app/RefCam2/RefCam2.apk" 
adb shell "rm /system/app/Camera2/Camera2.apk" 
adb shell "rm /system/app/MSMTest/MSMTest.apk" 
adb shell "rm /system/app/MSMClient/MSMClient.apk" 
adb shell "rm /system/app/ArcCamera/ArcCamera.apk"

#Run monkey in a loop
_cnt=1
while true; do
    log_date "[$_cnt]"
    _cnt=$(($_cnt + 1))

        # Unlock
        adb shell 'input keyevent' 3 #home
        # Execute monkey
        log_date "[monkey -v $_ninputs]"
        adb shell " monkey -v-v --ignore-crashes --ignore-timeouts --kill-process-after-error --ignore-security-exceptions --throttle 1000 -v $_ninputs">> "$_log_file"
        # Wait for phone to settle in case of system_server crash
        sleep 5
        adb wait-for-devices
        sleep 15
done

