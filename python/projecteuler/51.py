#!/usr/bin/python

import euler 
import time

def gen_num(a, b, c, d, e, f, ma, mb, mc, md, me, r):
    result = 0
    if ma:  result += r * 100000
    else:   result += a * 100000
    if mb:  result += r * 10000
    else:   result += b * 10000
    if mc:  result += r * 1000
    else:   result += c * 1000
    if md:  result += r * 100
    else:   result += d * 100
    if me:  result += r * 10
    else:   result += e * 10
    result += f
    return result


def problem():
    for mask in xrange(32):
	ma = (mask & 16) // 16
	mb = (mask & 8) // 8
	mc = (mask & 4) // 4
	md = (mask & 2) // 2
	me = (mask & 1) 
	if ma + mb + mc + md + me < 2:
#	    print ma, mb, mc, md, me
	    continue
	
	for a in xrange(10 - 9 * ma):
	    for b in xrange(10 - 9 * mb):
		for c in xrange(10 - 9 * mc):
		    for d in xrange(10 - 9 * md):
			for e in xrange(10 - 9 * me):
			    for f in [1, 3, 7, 9]:
				result = []
				for r in xrange(1, 10):
				    num = gen_num(a, b, c, d, e, f, ma, mb, mc, md, me, r)
				    if euler.is_prime(num):
					result.append(num)
				if len(result) > 7:
				    print result
				    return

t = time.clock()
problem()
print time.clock() - t
