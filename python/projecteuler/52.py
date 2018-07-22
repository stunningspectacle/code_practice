#!/usr/bin/python

import time

def problem52(x):
    for i in xrange(10 ** (x - 1), 20 ** (x - 1)):
	x = list(str(i))
	x.sort()
	x2 = list(str(2 * i))
	x2.sort()
	if x != x2:
	    continue
	x3 = list(str(3 * i))
	x3.sort()
	if x != x3:
	    continue
	x4 = list(str(4 * i))
	x4.sort()
	if x != x4:
	    continue
	x5 = list(str(5 * i))
	x5.sort()
	if x != x5:
	    continue

	x6 = list(str(6 * i))
	x6.sort()
	if x != x6:
	    continue
	return i
    return False

for i in xrange(2, 10):
    num = problem52(i)
    if num:
	print num, 2*num, 3*num, 4*num, 5*num, 6*num
	break

