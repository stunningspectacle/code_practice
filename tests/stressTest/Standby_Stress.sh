adb -s $1 root
sleep 3
adb -s $1 shell "echo Y > /sys/module/kernel/parameters/initcall_debug"
adb -s $1 shell "echo 8 > /proc/sys/kernel/printk"
adb -s $1 shell "echo N > /sys/module/printk/parameters/console_suspend"
while true
do
adb -s $1 shell "ping -c 1 $2"
date
sleep 3
#adb -s $1 shell "ping -c 3 $2"
#sleep 5
done
