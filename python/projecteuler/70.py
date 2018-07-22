#!/usr/bin/python

import euler
import time

TOP = 10 ** 7

def prob70():
    primes = euler.primes_beforex(TOP // 2)
    len_primes = len(primes)
    min = (10, 0, 0)
    for i in xrange(len_primes):
	if i % 10000 == 0:
	    print "i=", i
	for j in xrange(i + 1, len_primes):
	    num = primes[i] * primes[j]
	    if num > TOP:
		break
	    str_num = [c for c in str(num)]
	    str_num.sort()
	    str_num = "".join(str_num)
	    phi = num - primes[i] - primes[j] + 1
	    str_phi = [c for c in str(phi)]
	    str_phi.sort()
	    str_phi = "".join(str_phi)
	    if str_num == str_phi:
		t = float(num)/phi
		if t < min[0]:
		    min = (t, num, phi)
		    print min, primes[i], primes[j]

t = time.clock()
prob70()
print time.clock() - t
