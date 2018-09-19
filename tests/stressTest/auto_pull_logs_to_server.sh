#/bin/sh
date=`date "+%Y-%m-%d"`
devices=`adb devices|grep -w device|awk {'print $1'}`

echo $1

if [ -z $1 ]
then
echo Please input test case name as 1st parameter!
exit
fi

echo start to collecting log on $1 test case $devices 

mkdir $date
pushd $date

for device in $devices;
do
	echo processing device $device;
	mkdir $device$1
	pushd $device$1

	adb -s $device wait-for-device
	adb -s $device root
	adb -s $device wait-for-device
	adb -s $device pull /data/logs/history_event
	grep crashlog history_event|grep -v JAVA|grep -v ANR|awk '{n=split($5,array, "/"); adb_cmd="adb -s " device " pull " $5 " " array[n] ; if(system( "test -d "array[n] ) == 0) { print array[n] " is exist"} else {print adb_cmd;system(adb_cmd)}}' device=$device
	for i in {1..20} 
	do
		adb -s $device pull /data/logs/aplog.$i 
	done
	adb -s $device pull /data/logs/aplog 
	adb -s $device pull /mnt/extSdCard/Coredump
	popd
done
popd 

echo "sync the logs to server..."

#rsync -avz --progress $date mtbf@lizhuangzhi-PC.sh.intel.com:/mtbf_logs/



