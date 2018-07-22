#!/usr/bin/python

import time

start = time.time()

def get_nchain(num):
    count = 1
    while True:
	if num == 1:break
	if num % 2 == 0:
	    num = num / 2
	else:
	    num = 3 * num + 1
	count += 1
    return count

def get_lchain(num):
    max = 0
    number = 0
    for i in xrange(1, num + 1):
	chain = get_nchain(i)
	if chain > max:
	    max = chain
	    number = i
	    print number, max
    return (number, max)

number, max = get_lchain(1000000)
print number, max


end = time.time()

print "Consume %f seconds" % (end - start)
