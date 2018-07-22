#!/usr/bin/python

import euler

primes = euler.primes_beforex(1000000)

max = 0
max_seq = []
n_primes = len(primes)

for i in xrange(n_primes - 1):
    if i % 1000 == 0:
	print "i=", i
    val = i 
    p_sum = primes[i]
    seq = [primes[i]]
    result = []
    while p_sum < 1000000:
	val += 1
	p_sum += primes[val]
	seq.append(primes[val])
	if euler.is_prime(p_sum):
	    result = seq[:]

    if max < len(result):
	max = len(result)
	max_seq = result[:]


print max_seq
print len(max_seq), "primes, sum:", sum(max_seq)

