dev=$1

counter=0
while true
do

counter=$(( $counter + 1 ))
	echo "@@@@@@@@"loop $counter "@@@@@@@@"
	date


adb -s $dev wait-for-device
adb devices
adb -s $dev root

sleep 60
adb -s $dev shell ls /mnt/media_rw/*/Coredump/
adb -s $dev shell cat /data/logs/history_event | grep -i tombstone
adb -s $dev reboot	
done
