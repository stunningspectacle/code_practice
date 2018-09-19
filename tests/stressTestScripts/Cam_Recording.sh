echo "N" > /sys/module/printk/parameters/console_suspend
echo 8 > /proc/sys/kernel/printk
while true
do
echo "*********************"
pkill camera
find /mnt/shell/emulated -name *3gp* -exec rm {} \;

am start -a android.intent.action.MAIN -n com.android.camera2/com.android.camera.CameraLauncher
input keyevent 27
sleep 600
done

