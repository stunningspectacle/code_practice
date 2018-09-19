#!/system/bin/sh

#RST-128
#INT-133

echo nopull > /d/gpio_debug/gpio84/current_pullmode 
echo nopull > /d/gpio_debug/gpio85/current_pullmode

echo pullup > /d/gpio_debug/gpio133/current_pullmode 
echo pullup > /d/gpio_debug/gpio128/current_pullmode 

sleep 1
echo high > /d/gpio_debug/gpio128/current_value
sleep 1
echo low > /d/gpio_debug/gpio128/current_value
echo low > /d/gpio_debug/gpio133/current_value
sleep 1

echo high > /d/gpio_debug/gpio133/current_value
sleep 1

echo high > /d/gpio_debug/gpio128/current_value
sleep 1

echo nopull > /d/gpio_debug/gpio133/current_pullmode

