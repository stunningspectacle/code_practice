#!/usr/bin/python
import sys

num=[]

if (len(sys.argv) < 2):
    print "No action"
    exit(0)
    
n = int(sys.argv[1])
if n < 2:
    print "too small"
    exit(0)

num = [i for i in xrange(0, n)]
mark = [0 for i in xrange(0, n)]

for i in xrange(2, n):
    if mark[i] != 0:
	continue
    j = i + i
    while j < n: 
	mark[j] += 1
	j += i

prime = [i for i in xrange(2, n) if mark[i] == 0]

print prime
