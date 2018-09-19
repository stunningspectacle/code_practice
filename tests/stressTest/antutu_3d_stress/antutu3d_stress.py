#!/usr/bin/python

import time, os
from subprocess import *

os.system('adb -s 0123456789012345 wait-for-devices')

i=0
while True:
    res = check_output(["adb",  "-s 0123456789012345", "shell", "dumpsys activity | grep Native3DLoader"])
    print len(res)
    if len(res) == 0:
        os.system('adb -s 0123456789012345 shell am start -n com.antutu.ABenchMark/com.antutu.benchmark.test3d.Antutu3DLoader')   
        time.sleep(5)        
        i=i+1
        print "count ", i
    time.sleep(1)
print "======= exit========="

