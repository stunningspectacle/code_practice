adb -s $1 root
sleep 3
adb -s $1 remount
adb -s $1 shell "echo Y > /sys/module/kernel/parameters/initcall_debug"
adb -s $1 shell "echo 8 > /proc/sys/kernel/printk"
adb -s $1 shell "echo N > /sys/module/printk/parameters/console_suspend"
#adb -s $1 shell "echo processors > /sys/power/pm_test"
#adb -s $1 shell "echo platform > /sys/power/pm_test"
adb -s $1 shell "echo core > /sys/power/pm_test"
