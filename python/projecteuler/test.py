#!/usr/bin/python

t_l = [1, 2, 3, 4]

def xperm(list, n):
    if n < 1:
	yield []
    for i in xrange(len(list)):
	for j in xperm(list[:i] + list[i+1:], n-1):
	    yield [list[i]] + j

print [a for a in xperm(t_l, 4)]
    

