#!/usr/bin/python

def prob56():
    max = 0
    for a in xrange(1, 100):
	for b in xrange(1, 100):
	    num = a ** b
	    d_sum = sum(int(c) for c in str(num))
	    if d_sum > max:
		max = d_sum
		print max, a, b
    
    print max

prob56()
