#!/usr/bin/python

def add_fraction(int_part, frac_list):
    denom = frac_list[-1] 
    numer = 1
    frac_list = frac_list[:-1]
    for f in frac_list[::-1]:
	new_denom = numer + denom * f
	numer = denom
	denom = new_denom
    numer += denom * int_part
    return (numer, denom)

def prob65():
    e_frac = [1]
    for i in xrange(1, 35):
	e_frac += [2 * i, 1, 1]
    numer, denom = add_fraction(2, e_frac[0:99])
    print numer, denom
    print sum([int(c) for c in str(numer)])

prob65()
    


