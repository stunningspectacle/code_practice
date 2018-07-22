#!/usr/bin/python

import euler
import time

def prob68():
    digits = range(1, 10)
    perms = euler.xperm(digits, len(digits))
    for p in perms:
	num = p[0] + p[1] + 10
	if (p[5] + p[1] + p[2] == num and
	    p[6] + p[2] + p[3] == num and
	    p[7] + p[3] + p[4] == num and
	    p[8] + p[4] + p[0] == num):
	    print "%d%d%d,%d%d%d,%d%d%d,%d%d%d,%d%d%d" % (
		    10, p[0], p[1], 
		    p[5], p[1], p[2], 
		    p[6], p[2], p[3], 
		    p[7], p[3], p[4], 
		    p[8], p[4], p[0]), num

t = time.clock()
prob68()
print time.clock() - t

