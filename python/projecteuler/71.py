#!/usr/bin/python

def hcf(a, b):
    while b != 0:
	a, b = b, a % b
    return abs(a)

def prob71():
    TOP = 10 ** 3
    up_numer, up_denom = 1, 0
    low_numer, low_denom = 0, 1
    max = 0
    while True:
	n = up_numer + low_numer
	d = up_denom + low_denom
	if d > TOP: break
	if hcf(n, d) == 1:
	    print n, d
	if 2 * n >=  d:
	    up_numer, up_denom = n, d
	else:
	    low_numer, low_denom = n, d
	

prob71()
