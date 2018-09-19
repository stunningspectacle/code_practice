#!/usr/bin/python
import string
'''
 nt35596_cmd6 {
	 intel,cmd-type = <0x15>;
	 intel,cmd-data = <0x01 0x55>;
	 intel,cmd-lp = <1>; 
	 };   
'''
cmd_count = 0
arg0 = 0
arg1 = 0
param = 2
src = open("01_TXDT500BZPA_NT35596_196_2power.C", "r");
if not src:
    print "open src fail!"
    exit()
dst = open("NT35596_cmd.dts", "w")
if not dst:
    print "open dst fail!"
    exit()

def output_cmd(count, arg0, arg1):
    s = "nt35596_cmd" + str(count) + ''' {
	     intel,cmd-type = <0x15>;
	     intel,cmd-data = <''' + arg0 + " " +  arg1 + '''>;
	     intel,cmd-lp = <1>;
	 };
    '''
    dst.write(s)

while True:
    line = src.readline()
    if not line:
	break
    if line.startswith("SSD2828_WritePackageSize"):
	if param != 2:
	    print "command count", cmd_count
	    print "param is", param, ", something wrong in WritePackageSize!"
	    break
	param = 0
	continue
    if line.startswith("SPI_WriteData"):
	strs = line.split(';')
	if param == 0:
	    arg0 = strs[0].lstrip("SPI_WriteData(").rstrip(")")
	    param += 1
	elif param == 1:
	    arg1 = strs[0].lstrip("SPI_WriteData(").rstrip(")")
	    param += 1

	    output_cmd(cmd_count, arg0, arg1)
	    cmd_count += 1
	    #print "command count: ", cmd_count, "args:", arg0, ", ", arg1
	else:
	    print "command count: ", cmd_count, ", param:", param, ", something is wrong"
	    break
