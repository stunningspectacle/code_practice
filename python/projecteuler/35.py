#!/usr/bin/python

import time
import math

def is_prime(val):
    if val < 2: 
	return False
    st = int(math.sqrt(val)) + 1
    for x in xrange(2, st):
	if val % x == 0:
	    return False
    return True


start = time.time()
result = []

for i in xrange(2, 1000000):
    if is_prime(i):
	s = str(i)
	mark = 0
	for j in xrange(len(s) - 1):
	    s = s[1:] + s[0]
	    if not is_prime(int(s)):
		mark = 1
		break
	if mark == 0:
	    result.append(i)
	    print i,

print result
print len(result)

end = time.time()
print "Time consume:", end-start, "seconds"


