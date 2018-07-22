#!/usr/bin/python

import euler

result = []
num = 11
while True:
    if num % 100000 == 0:
	print "Now num=%d" % num
    if euler.is_prime(num):
	tmp1 = tmp2 = num
	mark = 0
	for i in xrange(len(str(num)) - 1):
	    s1 = str(tmp1)
	    tmp1 = int(s1[1:])
	    if not euler.is_prime(tmp1):
		mark = 1
		break
	    s2 = str(tmp2)
	    tmp2 = int(s2[:-1])
	    if not euler.is_prime(tmp2):
		mark = 1
		break
	if mark == 0:	
	    result.append(num)
	    if len(result) == 11:
		break
	    print num,
    num += 1

print result
print sum(result)
	


