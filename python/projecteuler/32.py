#!/usr/bin/python

def is_pand(a, b):
    c = a * b
    s = str(a) + str(b) + str(c)
    si = [int(c) for c in s]
    si.sort()
    s = "".join(str(i) for i in si)
    if s == "123456789":
	return True
    else:
	return False

pand = []
val = []

for a in xrange(1, 10):
    for b in xrange(1000, 10000):
	if is_pand(a, b):
	    pand.append((a, b, a*b))
	    if a*b not in val:
		val.append(a*b)

for a in xrange(10, 100):
    for b in xrange(100, 1000):
	if is_pand(a, b):
	    pand.append((a, b, a*b))
	    if a*b not in val:
		val.append(a*b)

print pand
print len(pand)
print val
print sum(val)

