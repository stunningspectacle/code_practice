#!/usr/bin/env bash

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
adb -s $_id root
sleep 2
adb -s $_id wait-for-devices

#RemoveInternalAPK
adb -s $_id remount
sleep 2
for apk in `cat internalAPKList.txt`
        do
                adb -s $_id shell rm $apk
        done

sleep 10

#Run monkey in a loop
_cnt=1
while true; do
    log_date "[$_cnt]"
    _cnt=$(($_cnt + 1))

        # Unlock
        adb -s $_id shell 'input keyevent' 3 #home
        # Execute monkey
        log_date "[monkey -v $_ninputs]"
        adb -s $_id shell " monkey -v-v --ignore-crashes --ignore-timeouts --kill-process-after-error --ignore-security-exceptions --throttle 1000 -v $_ninputs">> "$_log_file"
        # Wait for phone to settle in case of system_server crash
        sleep 5
        adb wait-for-devices
        sleep 15

done

