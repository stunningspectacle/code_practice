#!/usr/bin/python

def get_xy(d):
    low_numer, up_numer = 0, 1
    low_denom, up_denom = 1, 0
    while True:
	x = low_numer + up_numer
	y = low_denom + up_denom
	t = x * x - d * y * y
	if t == 0:
	    return None
	elif t == 1:
	    return (x, y, d)
	elif t > 1:
	    up_numer = x
	    up_denom = y
	else:
	    low_numer = x
	    low_denom = y

def prob66():
    TOP = 1001
    max = (0, 0, 0)
    for d in xrange(2, TOP):
	xyd = get_xy(d)
	if not xyd: continue
	if xyd[0] > max[0]:
	    max = xyd
    print max


prob66()


	


