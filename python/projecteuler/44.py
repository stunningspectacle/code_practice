#!/usr/bin/python
#2P = 3 n**2 - n
import math, time
import sys

t = time.clock()

def is_panta(num):
    tmp = 2 * num
    rt = math.sqrt(1 + 12 * tmp)
    if rt != int(rt):
	return False
    if (rt + 1) % 6 == 0:
	return True 
    return False


pantas = [1]
i = 5

while True:
    if is_panta(i):
	for pan in pantas:
	    if (is_panta(i - pan) and
		is_panta(i + pan)):
		print (i, pan, i - pan)
		print time.clock() - t
		sys.exit(0)
	pantas.append(i)
	print len(pantas)
    i += 1


