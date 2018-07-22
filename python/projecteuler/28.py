#!/usr/bin/python

count = 0
integer = 1
sum = 1

while True:
    edge = 2 * count + 1
    if edge == 1001: break

    for i in xrange(4):
	integer += edge + 1
	print integer,
	sum += integer
	
    count += 1

print "\n"
print sum


