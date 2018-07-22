#!/usr/bin/python

def nCr(n, r):
    numerator = reduce(lambda x, y: x*y, range(n-r+1, n+1))
    denominator = reduce(lambda x, y: x*y, range(1, r+1))
    return numerator/denominator

def problem53():
    count = 0
    for n in xrange(1, 101):
	if n % 10 == 0:
	    print "now n=", n
	for r in xrange(1, n):
	    if nCr(n, r) > 1000000:
		count += 1
    
    print count

problem53()
