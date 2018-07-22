#!/usr/bin/python

def expand(num):
    numer = 1
    denom = 2
    while num > 0:
	numer = numer + 2 * denom
	tmp = numer
	numer = denom
	denom = tmp
	num -= 1

    return (numer + denom , denom)

def prob57():
    collect = []
    for i in xrange(1000):
	numer, denom = expand(i)
	if len(str(numer)) > len(str(denom)):
	    collect.append((numer, denom))
	
    print collect
    print len(collect)


prob57()
