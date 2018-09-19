#!/system/bin/sh

cd /d/gpio_debug/gpio191
echo low > current_value

cd /d/gpio_debug/gpio191
echo high > current_value

sleep 1

echo "change sleep"
cd /sys/bus/i2c/devices/7-0020/rmi4
echo 1 > fn_num
echo 0 > reg_addr

echo "Sleep mode:"
cat ctrl_reg
echo 2 > ctrl_reg
cat ctrl_reg
echo 0 > ctrl_reg
cat ctrl_reg

echo "Calibration interval:"
echo 9 > reg_addr
cat ctrl_reg
echo 5 > ctrl_reg
cat ctrl_reg
#sleep 0.1
#echo 128 > ctrl_reg
#cat ctrl_reg

#i=0
#while [ $i -lt 4 ]; do
#    echo $i > reg_addr
#    v=$(cat data_reg)
#    echo "i=$i, value=$v"
#    i=`expr $i + 1`
#done
