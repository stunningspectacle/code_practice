#!/usr/bin/python

import euler
import string

def replace_prime(prime):
    s_prime = str(prime)
    for rc in ["0", "1", "2"]:
	primes = []
	if s_prime[:-1].count(rc) < 2:
	    continue
	for digit in string.digits[1:]:
	    tmp = s_prime[:-1].replace(rc, digit) + s_prime[-1]
	    if euler.is_prime(int(tmp)):
		primes.append(int(tmp))
	if len(primes) > 7:
	    return primes
	else:
	    return None
    

def problem51():
    primes = euler.primes_beforex(1000000)
    for x in primes:
	if primes < 10000: continue
	for x in primes:
	    prime_list = replace_prime(x)
	    if prime_list:
		print prime_list
		return

import time
t = time.clock()
problem51()
print time.clock() - t
