#!/usr/bin/python
import re

#f = open("ctp.log")
#log = f.read()
#lux = re.findall("lux_clear=(\d+), lux_ir=(\d+)", log)
#print "\n".join(ch0 + " " + ch1 + " " + str(float(int(ch1))/int(ch0)) for ch0, ch1 in lux)

RATIO_RANGE0 = 45
RATIO_RANGE1 = 64
RATIO_RANGE2 = 85

CH0_COEFF_RANGE0 = 17743
CH1_COEFF_RANGE0 = 11059                                                                 
CH0_COEFF_RANGE1 = 37725                                                                
CH1_COEFF_RANGE1 = -13363
CH0_COEFF_RANGE2 = 2075
CH1_COEFF_RANGE2 = 960

WINDOW_COMP = 40
COEFF_SCALE = 10000

RATIO_RANGE0 = 45
RATIO_RANGE1 = 71
RATIO_RANGE2 = 95       

f = open("ltr301.log")
data = []
while True:
	line = f.readline()
	if line == "":
		break
	for ch0, ch1, lux in re.findall("ch0=(\d+), ch1=(\d+), lux=(\d+)", line):
		ch0, ch1, lux = int(ch0), int(ch1), int(lux)
		if ch0 + ch1 == 0:
			continue
		ratio = ch1 * 100 / (ch0 + ch1)
		opacity, ch0_co, ch1_co = 1100, 0, 0
		mylux, wc = 0, 0
		if ratio < RATIO_RANGE0:
			ch0_coeff = CH0_COEFF_RANGE0
			ch1_coeff = CH1_COEFF_RANGE0
			wc = WINDOW_COMP
		elif ratio < RATIO_RANGE1: 
			ch0_coeff = CH0_COEFF_RANGE1 
			ch1_coeff = CH1_COEFF_RANGE1
		else:
			ch0_coeff = CH0_COEFF_RANGE2
			ch1_coeff = CH1_COEFF_RANGE2
		mylux = (ch0 * ch0_coeff + ch1 * ch1_coeff) * 7 / COEFF_SCALE
		if mylux >= wc:
		   mylux += wc

		data.append([lux, ch0, ch1, ratio, mylux])

data.sort()
print "%-10s%-10s%-10s%-10s%-10s" % ("lux", "ch0", "ch1", "ratio", "mylux")
for lux, ch0, ch1, ratio, mylux in data:
	print "%-10d%-10d%-10d%-10d%-10d" % (lux, ch0, ch1, ratio, mylux)
