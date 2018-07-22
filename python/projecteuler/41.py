#!/usr/bin/python

import euler

tail_num = ["1", "2", "3", "4", "5", "6", "7"]
head = ""
result = []
max = 0
for x in euler.xperm(tail_num, 7):
    num = int(head + "".join(x))
    if euler.is_prime(num):
	result.append(num)
	if num > max:
	    max = num
	    print num,

print "\n"
print result
print max

