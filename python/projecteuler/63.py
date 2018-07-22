#!/usr/bin/python

def n_exp(num):
    count = 0
    exp = 0
    while True:
	exp += 1
	if len(str(num ** exp)) < exp:
	    break
	if len(str(num ** exp)) == exp:
	    print num, exp, num**exp
	    count += 1
    return count

def prob63():
    result = []
    for num in xrange(1, 10):
	result.append(n_exp(num))
    print sum(result)


prob63()

	
