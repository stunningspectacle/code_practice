#!/usr/bin/python
import math
import time

def sqrt():
    hard = []
    t = time.clock()
    TOP = 100
    max = (0, 0)
    for d in xrange(2, TOP):
#	if d % 10 == 0:
	tmp = math.sqrt(d)
	if tmp == int(tmp):
	    continue
	z = d
	tt = time.clock()
	while True:
	    x = math.sqrt(z + 1)
	    y = math.sqrt(z / d)
	    if (x == int(x) and y == int(y)):
		if x > max[0]:
		    max = (x, d)
		print "%d*%d = %d*%d*%d + 1" % (x, x, d, y, y)
		break
	    z += d
	#    if time.clock() - tt > 1:
	#	hard.append(d)
	#	break
    print max
    print hard
    print "sqrt:", time.clock() - t

def prob66():
    t = time.clock()
    TOP = 61
    max = (0, 0)
    for d in xrange(2, TOP):
#	if d % 10 == 0:
	tmp = math.sqrt(d)
	if tmp == int(tmp):
	    continue
	x, y = 1, 1
	while True:
	    num_x = x * x
	    num_y = y * y * d + 1

	    if num_x == num_y:
		if x > max[0]:
		    max = (x, d)
	#	print "%d*%d = %d*%d*%d + 1" % (x, x, d, y, y)
		break
	    if num_x < num_y:
		x += 1
	    else:
		y += 1
    print max
    print "Inc:", time.clock() - t

sqrt()
