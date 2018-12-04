#!/usr/bin/python
import sys

def prime(MAX):
	mark = [0] * MAX

	for i in range(2, MAX/2):
		j = i * 2
		while j < MAX:
			mark[j] = 1
			j += i

	for i in range(2, MAX):
		if mark[i] == 0:
			yield i

if len(sys.argv) != 2:
    print "Usage:", sys.argv[0], "N"
    exit()

primes = []
num = int(sys.argv[1])
SUM = 0
for i in prime(num):
	SUM += i
	primes.append(i)

print primes
print SUM
