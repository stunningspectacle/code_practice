#!/usr/bin/python
import string
'''
 ili7807b_cmd6 {
	 intel,cmd-type = <0x15>;
	 intel,cmd-data = <0x01 0x55>;
	 intel,cmd-lp = <1>; 
	 };   
cmd78 {
	intel,cmd-type = <0x39>;
	intel,cmd-data = <0xC8 0x01 0x20>;
	intel,cmd-lp = <1>;
	};
'''

cmd_count = 0
src = open("TXDT500UZPA_ILI7807B_H497DAN02_Column_4lane_20160304.txt", "r");
if not src:
    print "open src fail!"
    exit()
dst = open("ILI7807B_cmd.dts", "w")
if not dst:
    print "open dst fail!"
    exit()

def output_cmd(count, cmd_type, cmd_data):
    s = "ili7807b_cmd" + str(count) + ''' {
	     intel,cmd-type = <''' + cmd_type + '''>;
	     intel,cmd-data = <''' + cmd_data + '''>;
	     intel,cmd-lp = <1>;
	 };
    '''
    dst.write(s)

while True:
    line = src.readline()
    if not line:
	break

    cmd_type = 0
    cmd_data = ""
    if line.startswith("{"):
	strs = line.strip().rstrip("},").replace("}", "").replace("{", "").split(',')
	if strs[1] == "1":
	    print "It's 0"
	    cmd_type = "0x15"
	elif strs[1] == "3":
	    print "It's 3"
	    cmd_type = "0x39"
	else:
	    print "Error, len is not 0 or 3"

	cmd_data = strs[0] + " " + string.join(strs[2:], " ")

	output_cmd(cmd_count, cmd_type, cmd_data)
	cmd_count += 1
	#print "command count: ", cmd_count, "args:", arg0, ", ", arg1
