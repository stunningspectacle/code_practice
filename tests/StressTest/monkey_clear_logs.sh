#/bin/sh
date=`date "+%Y-%m-%d"`
date=$date-r
echo $date
devices=`adb devices|grep -w device|awk {'print $1'}`

mkdir $date
pushd $date

#for device in $devices;
#do
#echo ----start to clean $devices logs-----
#adb -s $device shell "rm -rf /sdcard/logs/*"
#adb -s $device shell "rm -rf /data/logs/core/*"
#echo ----clean $devices logs done-----
#done

for device in $devices;
do
adb -s $device root
echo adb root $device
sleep 3
done
#adb -s $1 remount

mkdir ~/logs/$date
 
while true
do
#sleep 20
sleep 3600
for device in $devices;
do
echo pulling $device log
mkdir ~/logs/$date/$device
adb -s $device pull /sdcard/logs ~/logs/$date/$device
adb -s $device pull /data/logs/core ~/logs/$date/$device/core
sleep 3
adb -s $device shell "rm -rf /sdcard/logs/*"
adb -s $device shell "rm -rf /data/logs/core/*"
done
echo "sync the logs to server..."

rsync -avz --progress ~/logs/$date mtbf@lizhuangzhi-PC.sh.intel.com:/mtbf_logs/
done
