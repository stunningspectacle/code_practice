#!/bin/bash
###### Usage: ./stability_playmp4.sh $deviceID
device=$1
loop=3000
date='date +%Y-%m-%d\ %H:%M:%S'
LOG=stability_playmp4_${device}.log
cat /dev/null > $LOG
i=1
#adb -s $device root
echo 
while [ $i -le $loop ]
do
	echo
	date +%Y-%m-%d***%H:%M%S
	echo "LOOP COUNT=$i"
#	adb -s $device shell "echo 0 > /d/watchdog/kernel_watchdog/enable "
#	adb -s $device shell "echo 0 > /d/watchdog/vmm_scu_wdt_enable "
	echo "/d/watchdog/kernel_watchdog/enable=` adb -s $device shell "cat  /d/watchdog/kernel_watchdog/enable "`"
	echo "/d/watchdog/vmm_scu_wdt_enable=` adb -s $device shell "cat  /d/watchdog/vmm_scu_wdt_enable "`"
	adb -s $device shell "am start -a android.intent.action.VIEW -d file:////sdcard/Movies/H264_L4.0_MP_1920x1088_30fps_AAC+_ST_128kb_48KHz.mp4 -t video/*"
	#adb -s $device shell "am start -n com.google.android.apps.plus/com.google.android.apps.plus.phone.VideoViewActivity -d file:////sdcard/Movies/H264_L4.0_MP_1920x1088_30fps_AAC+_ST_128kb_48KHz.mp4 -t video/*"
	# if [ $? -eq 0 ]
	echo "play video $i times started at BEHCH time: `date` DUT time: `adb -s $device shell date`" >> $LOG
		# echo "${date}" >> $LOG
	# else
	# 	echo "play the video failed at $i times"
	# 	contiNUES
	sleep 63
	let i=i+1
done

#am start -n com.glbenchmark.glbenchmark27/com.glbenchmark.activities.GLBenchmarkDownloaderActivity
#am start -n com.glbenchmark.glbenchmark27/com.glbenchmark.activities.GLBMainMenu
#am start -n com.glbenchmark.glbenchmark27/com.glbenchmark.activities.GLBPerformanceTestSelector
#am start -n com.glbenchmark.glbenchmark27/com.glbenchmark.activities.GLBRender
