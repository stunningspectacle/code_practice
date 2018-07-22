#!/usr/bin/python

from math import sqrt

num = 2
count = 0
while True:
    num2 = int(sqrt(num)) + 1
    mark = 0
    for i in xrange(2, num2):
	if num % i == 0:
	    mark = 1
	    break
    if mark == 0:
#	print num,
	count += num
	print count,
    num += 1
    if num == 2000000:break
    


