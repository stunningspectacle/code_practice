#!/usr/bin/python

import re
import math

findcircle = re.compile(r"(\d+)\1").findall
max = 0
for i in xrange(2, 1000):
    digit_len = len(str(i))
    scale = 10**digit_len
    frac = ""
    numer = 1 * scale
    while True:
	part = ""
	for x in xrange(10):
	    part = part + str(numer // i)
	    numer = (numer % i) * scale
	if part in frac:
	    index = frac.find(part)
	    tail = frac[index:]
	    frac = frac + tail
	    break
	frac = frac + part
    circle = findcircle(frac)
    real_circle = ""
    for s in circle:
	if len(s) > len(real_circle):
	    real_circle = s
    print "1/%d len = %d: %s" % (i, len(real_circle), real_circle)
    if len(real_circle) > max:
	max = len(real_circle)
	print "max %d: 1/%d:%s" % (max, i, real_circle)
	

