#!/usr/bin/python


import math
import euler

(   TRIANGLE,
    SQUARE,
    PENTA,
    HEXA,
    HEPTA,
    OCTA
) = range(6)


def is_triangle(num): 
    """Triangle	    P_(3,n)=n(n+1)/2	1, 3, 6, 10, 15, ..."""
    sr = math.sqrt(8 * num + 1)
    if int(sr) != sr:
	return False
    if (sr - 1) % 2 == 0:
	return True
    return False

def is_square(num):
    """Square	    P_(4,n)=n^(2)	1, 4, 9, 16, 25, ..."""
    sr = math.sqrt(num)
    if int(sr) != sr:
	return False
    return True

def is_penta(num):
    """Pentagonal	P_(5,n)=n(3n-1)/2	1, 5, 12, 22, 35, ..."""
    sr = math.sqrt(24 * num + 1)
    if int(sr) != sr:
	return False
    if (sr + 1) % 6 == 0:
	return True
    return False

def is_hexa(num):
    """Hexagonal	P_(6,n)=n(2n-1)		1, 6, 15, 28, 45, ..."""
    sr = math.sqrt(8 * num + 1)
    if int(sr) != sr:
	return False
    if (sr + 1) % 4 == 0:
	return True
    return False

def is_hepta(num):
    """Heptagonal	P_(7,n)=n(5n-3)/2	1, 7, 18, 34, 55, ..."""
    sr = math.sqrt(40 * num + 9)
    if int(sr) != sr:
	return False
    if (sr + 3) % 10 == 0:
	return True
    return False

def is_octa(num):
    """Octagonal	P_(8,n)=n(3n-2)		1, 8, 21, 40, 65, """
    sr = math.sqrt(12 * num + 4)
    if int(sr) != sr:
	return False
    if (sr + 2) % 6 == 0:
	return True
    return False


def prob61():
    sets = [[] for i in xrange(6)]
    for i in xrange(1000, 10000):
	if is_triangle(i):
	    sets[TRIANGLE].append(i)
	if is_square(i):
	    sets[SQUARE].append(i)
	if is_penta(i):
	    sets[PENTA].append(i)
	if is_hexa(i):
	    sets[HEXA].append(i)
	if is_hepta(i):
	    sets[HEPTA].append(i)
	if is_octa(i):
	    sets[OCTA].append(i)
   
    num_list = euler.xperm(range(6), 6)
    for order in num_list:
	for a in sets[order[0]]:
	    for b in sets[order[1]]:
		if str(a)[2:] != str(b)[:2]:
		    continue
		for c in sets[order[2]]:
		    if str(b)[2:] != str(c)[:2]:
			continue
		    for d in sets[order[3]]:
			if str(c)[2:] != str(d)[:2]:
			    continue
			for e in sets[order[4]]:
			    if str(d)[2:] != str(e)[:2]:
				continue
			    for f in sets[order[5]]:
				if str(e)[2:] != str(f)[:2]:
				    continue
				if str(f)[2:] == str(a)[:2]:
				    result = [a, b, c, d, e, f]
				    print result, sum(result)


prob61()
