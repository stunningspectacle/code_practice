#!/usr/bin/python
import time

def is_xprime(num, x):
    count = 0
    previous = 0
    i = 1
    while True:
	i += 1
	if num % i == 0:
	    num = num / i
	    if i != previous:
		count += 1
	    previous = i
	    i -= 1
	if count > x:
	    return False
	if num == 1: break
    if count == x:
	return True
    return False

def check_xprime_range(num, x):
    count = 0
    for i in xrange(num - x + 1, num + x):
	if is_xprime(i, x):
	    count += 1
	    if count == x:
		return True
	else:
	    count = 0
    return False

t = time.clock()
i = 600
step = 4
while True:
    if i % 1000 == 0:
	print "i=", i
    if is_xprime(i, step):
	if check_xprime_range(i, step):
	    print i, 
	    break
    i += step

for x in xrange(i - step + 1, i + step):
    if is_xprime(x, step):
	print x,

print time.clock() - t
