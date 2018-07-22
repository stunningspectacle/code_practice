#!/usr/bin/python

import string
import euler

digit_list = [c for c in string.digits]

result = []
orig = []
for x in euler.xperm(digit_list, 3):
    num = int("".join(c for c in x))
    if num % 17 == 0:
	orig.append(num)

for num in orig:
    dl = digit_list[:]
    print "Now num = %d" %  num
    if num < 100:
	dl.remove("0")
    for c in str(num):
	dl.remove(c)
    for x in euler.xperm(dl, 7):
	s_num = "".join(c for c in x) + str(num)
	if (int(s_num[1:4]) % 2 == 0 and
	    int(s_num[2:5]) % 3 == 0 and
	    int(s_num[3:6]) % 5 == 0 and
	    int(s_num[4:7]) % 7 == 0 and
	    int(s_num[5:8]) % 11 == 0 and
	    int(s_num[6:9]) % 13 == 0):
		result.append(s_num)
		print s_num,


print result
print sum(int(s) for s in result)
