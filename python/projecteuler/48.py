#!/usr/bin/python

def nn_lastx_digits(n, x):
    num = n
    for i in xrange(n-1):
	num = num * n
	if len(str(num)) > x:
	    num = int(str(num)[-x:])
    
    return num

print sum(nn_lastx_digits(i, 10) for i in xrange(1, 1001))

