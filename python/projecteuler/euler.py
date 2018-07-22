#!/usr/bin/python

import math

def dec2bin(num):
    if num == 0:
	return "0"
    bin = []
    while True:
	if num == 0: break
	num, mod = divmod(num, 2)
	bin.append(mod)
    return "".join(str(i) for i in bin[::-1])

def is_prime(num):
    if num < 2: return False
    if num == 2: return True
    if num % 2 == 0: return False
    divisor = 3
    limit = num
    while limit > divisor:
	if num % divisor == 0:
	    return False
	limit = num // divisor
	divisor += 2
    return True

def primes_beforex(Limit):
    primes = []
    factor = [0] * Limit
    for i in xrange(2, Limit):
	if factor[i] == 0:
	    primes.append(i)
	    val = i
	    while val < Limit:
		factor[val] += 1
		val += i
    return primes

def get_nprimes(n):
    if n == 1: return [2]
    primes = [2]
    a = 3
    i = 0
    while True:
	sr = math.sqrt(a)
	mark = 0
	for p in primes:
	    if a % p == 0:
		mark = 1
		break
	    if p > sr: break
	if not mark:
	    primes.append(a)
	    if len(primes) == n:
		return primes
	a += 2


def get_combine(list, num):
    mark = [1] * num
    length = len(list)
    mark.extend([0] * (length - num))
    combine_list = []
    while True:
	combine_list.append([list[i] for i in xrange(len(mark)) if mark[i] == 1])
	try:
	    for i in xrange(length):
		if mark[i] == 1 and mark[i + 1] == 0:
		    mark[i] = 0 
		    mark[i + 1] = 1
		    count_1 = 0
		    for j in xrange(i+1):
			if mark[j] == 1:
			    count_1 += 1
			    mark[j] = 0
		    for j in xrange(count_1):
			mark[j] = 1
		    changed = 1
		    break
	except:
	    break
    return combine_list

def xperm(list, n):
    if n == 0:
	yield []
    else:
	for i in xrange(len(list)):
	    for xx in xperm(list[0:i]+list[i+1:], n-1):
		yield [list[i]] + xx

def get_prime_factors(num):
    root = num
    prime_factors = []
    i = 1
    while True:
	i += 1
	if num % i == 0:
	    num = num / i
	    prime_factors.append(i)
	    i -= 1
	if num == 1: break
    
    return prime_factors
