#!/usr/bin/python
import time

def prob72():
    TOP = 10 ** 6
    phis = [0, 0] + range(2, TOP + 1)
    count = 0
    for i in xrange(2, TOP + 1):
	if phis[i] != i: 
	    count += phis[i]
	    continue
	phis[i] -= 1
	count += phis[i]
	val = i + i
	while val <= TOP:
	    phis[val] = (phis[val] // i) * (i - 1)
	    val += i
    print count

t = time.clock()

prob72()

print time.clock() - t
