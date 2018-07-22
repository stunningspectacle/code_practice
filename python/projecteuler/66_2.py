#!/usr/bin/python

def test(n):
    n1, n2 = 0, 1
    d1, d2 = 1, 0
    while True:
	a = n1 + n2
	b = d1 + d2
	t = a * a - n * b * b
	if t == 1:
	    return a
	elif t == 0:
	    print "error"
	    return 0
	elif t > 1:
	    n2 = a
	    d2 = b
	else:
	    n1 = a
	    d1 = b

max = (0, 0)
for i in xrange(2, 101):
    x = test(i)
    if x == 0:
	continue
    print (x, i)
    if x > max[0]:
	max = (x, i)


print max


