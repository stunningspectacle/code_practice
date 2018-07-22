#!/usr/bin/python

import euler

primes = euler.primes_beforex(10000)

for i in xrange(len(primes)):
    if primes[i] > 1000:
	primes = primes[i:]
	break
final = []
for i in xrange(len(primes)):
    pl = [c for c in str(primes[i])]
    perm = euler.xperm(pl, 4)
    result = []
    for com in perm:
	com_num = int(com[0] + com[1] + com[2] + com[3])
	if (com_num in primes[i+1:] and
	    com_num not in result):
	    result.append(com_num)
    if len(result) > 2:
	final.append(result)

for com in final:
    com.sort()
    for i in xrange(len(com) - 2):
	if com[i+1] - com[i] == com[i+2] - com[i+1]:
	    print com[i], com[i+1], com[i+2]
    
