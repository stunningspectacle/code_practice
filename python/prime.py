#!/usr/bin/python

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

SUM = 0
for i in prime(100000):
	SUM += i

print SUM
print sum([i for i in prime(100000)])
	

