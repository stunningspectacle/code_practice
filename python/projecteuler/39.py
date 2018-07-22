#!/usr/bin/python

import time

def get_right_triangles(num):
    triangles = []
    for a in xrange(1, num):
	for b in xrange(a, num):
	    y = a + b
	    if y >= num : break
	    if y <= num - y:continue
	    x = a * b
	    if num * num + 2 * x == 2 * num * y:
		triangles.append((a, b, num - a - b, num))
    return triangles

start = time.time()
for i in xrange(1001):
    triangles = get_right_triangles(i)
    if len(triangles) > 0:
	print triangles

end = time.time()
print "Time consumed:", end-start
		
