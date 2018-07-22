#!/usr/bin/python


def get_pan(num):
    n = 1
    result = ""
    while True:
	s = str(num * n)
	if "0" in s:
	    return 0
	for c in s:
	    if c in result:
		if len(result) == 9:
		    return int(result)
		else:
		    return 0
	    if s.count(c) > 1:
		return 0
	result += s
	n += 1


max = 0

for i in xrange(11, 10000):
    num = get_pan(i)
    if num != 0:
	if num > max:
	    print (i, num)
	    max = num

print max



