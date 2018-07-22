#!/usr/bin/python

import euler

def check_set(set, num):
    for i in set:
	if not euler.is_prime(int(str(i) + str(num))):
	    return False
	if not euler.is_prime(int(str(num) + str(i))):
	    return False
    return True

def prob60():
    TOP = 10000
    BOTTOM = 100
    primes = euler.primes_beforex(TOP)
    heads = euler.primes_beforex(BOTTOM)
    head = 5
    n_primes = len(primes)
    set = [heads[head]]
    i = primes.index(heads[head]) + 1
    min = 100000
    while True:
	if check_set(set, primes[i]):
	    set.append(primes[i])
	    print set
	if len(set) == 5:
	    tmp = sum(set)
	    print set, tmp
	    if tmp < min:
		min = tmp
	    i = primes.index(set[-1])
	    del set[-1]
	    break
	i += 1
	if i == n_primes:
	     i = primes.index(set[-1]) + 1
	     del set[-1]
	     if len(set) == 0:
		 head += 1
		 print "head=", heads[head]
		 if head == len(heads):
		     break
		 set = [heads[head]]
		 i = primes.index(heads[head]) + 1
    print min

prob60()
	
