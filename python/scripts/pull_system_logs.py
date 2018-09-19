import sys, os
import time, re

LogFolder = ""

os.system("adb root")
os.system ("adb remount")
time.sleep(3)

def getDeviceList():
    ret = ''
    filename = 'devices.txt'
    os.system ("adb devices > " + filename)
    cfile = open (filename, "r")
    loglines = cfile.readlines()
    cfile.close()
   # print loglines
    rDevice = re.compile('(\w+)(\s+)(device)', re.IGNORECASE)
    for line in loglines :
        if rDevice.match(line) != None:
            ret = ret + line
    ret = ret[0:-1]
    return ret

print ">>>>>> Detecting adb devices! <<<<<<\n"
str_adbdevices = getDeviceList()

array_adbdevices = str_adbdevices.split('\n')
i_adbdevices = len(array_adbdevices)
if(i_adbdevices != 1):
    print ">>>>>> ERROR: Haven't found adb devices! <<<<<<\n"
    exit(0)

for i in range(0, len(sys.argv)) :
    if (sys.argv[i] == "-f"):
        LogFolder = sys.argv[i + 1]
    elif (sys.argv[i] == "help"):
        printHelp()
        exit(0)
  
print "------ ------ ------ ------ ------ ------ \n"
print "!!!!!!! Start catch system logs !!!!!!!\n"
# Generate log folder automatically if haven't named yet. 
if LogFolder == '':
    STARTDATE = time.strftime('%Y-%m-%d',time.localtime(time.time()))
    STARTTIME = time.strftime('%H.%M.%S',time.localtime(time.time()))
    LogFolder = os.path.join("monkeylogs", STARTDATE , STARTTIME)
    
print ">>>>>> Create folder LogFolder! <<<<<<\n"
os.makedirs(LogFolder)
os.chdir(LogFolder)

os.system("adb pull /sdcard/logs sdcard_logs")
os.system("adb shell rm -r /sdcard/logs/*")
os.system("adb pull /data/logs data_logs")
os.system("adb shell rm -r /data/logs/*")
os.system("adb pull /data/tombstones tombstones")
os.system("adb shell rm -rf /data/tombstones/*")
os.system("adb pull /data/dontpanic dontpanic")
os.system("adb shell rm -r /data/dontpanic/*")
os.system("adb pull /data/anr anr")
os.system("adb shell rm -r /data/anr/*")