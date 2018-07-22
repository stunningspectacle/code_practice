#!/usr/bin/python
import euler

def prob58():
    layer = 2
    count = 1
    nums = [1]
    primes = []
    inc = 0
    while True:
	inc = 2 * layer - 2
	for i in xrange(4):
	    count += inc
	    nums.append(count)
	    if euler.is_prime(count):
		primes.append(count)
	if ((len(primes) * 10) // len(nums)) < 1:
	    print inc + 1
	    return
	layer += 1

prob58()
