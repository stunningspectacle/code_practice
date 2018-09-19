date=`date +"%y%m%d%H"`
echo $date
sleep 2
mkdir monkey_log
while true
do
	adb -s $1 wait-for-device root
	adb -s $1 remount
	sleep 2
#	adb -s $1 shell "logcat -b radio -b events -b main -b system -v time " >> monkey_log/monkey_logcat_$1_$date.txt &
	echo xxxxx
	sleep 2
	adb -s $1 shell "monkey --pct-anyevent 0 --pct-trackball 0 --pct-flip 0 --ignore-crashes --ignore-timeouts --ignore-security-exceptions --monitor-native-crashes --ignore-native-crashes -v -v -v -s 1350985505710 --throttle 500 990000" >> monkey_log/MonkeyLogs_$1_$date.txt
	sleep 50
#	adb -s $1 shell "pkill logcat"
done
