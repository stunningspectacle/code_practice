#!/usr/bin/python
import euler

result = []

def is_palin(i):
    s = str(i)
    rev = s[::-1]
    if s != rev:
	return False
    bin = euler.dec2bin(i)
    rev = bin[::-1]
    if bin != rev:
	return False
    return True

for i in xrange(1, 1000000):
    if is_palin(i):
	result.append(i)
	print i, 

print "\n"
print result
print sum(result)
