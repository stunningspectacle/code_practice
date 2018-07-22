#!/usr/bin/python

import time, sys
import math
import euler

def is_goldbach(num):
    sr = int(math.sqrt(num//2))
    for i in xrange(1, sr + 1):
	if euler.is_prime(num - 2 * i * i):
	    return True
    return False


t = time.clock()
i = 4
while True:
    i += 1
    if i % 10000 == 0:
	print "i=", i
    n = 2 * i + 1
    if euler.is_prime(n):
	continue
    if not is_goldbach(n):
	print "compsite=", n
	break

print time.clock() - t
    
