#!/usr/bin/python

#fomula = n*n + a*n +b
import math

def is_prime(val):
    if val < 2: 
	return False
    st = int(math.sqrt(val)) + 1
    for x in xrange(2, st):
#	print "x = ", x
	if val % x == 0:
	    return False
    return True


max = 0
aa = 0
bb = 0
for a in xrange(-999, 1000):
    for b in xrange(-999, 1000):
	n = 0
	while True:
	    val = n*n + a*n + b
	    if not is_prime(val):
		break
	    else:
		n += 1
#	    print "after (a, b)"
	if n > 20:
	    print n, (a, b), a*b
	    if n > max:
		max = n
		aa, bb = a, b

print max, (aa, bb), aa*bb

