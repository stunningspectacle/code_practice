clear
device=$1
file=$2

cycle=1
while true;
do
echo ""
echo -n ">>>Test_Cycle $cycle uptime==> "
adb -s $device shell "uptime"; 

adb -s $device wait-for-device

sleep 15

cycle=$(( $cycle + 1));

date
echo -n "start video";
adb -s $device shell "am start -a android.intent.action.VIEW -d file:////sdcard/$file -t video/*"
sleep 100

done
