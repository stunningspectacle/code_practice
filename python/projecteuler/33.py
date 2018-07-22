#!/usr/bin/python

def is_cf(numer, denom):
    if numer >= denom or numer % 10 == 0:
	return False
    sn = str(numer)
    sd = str(denom)
    for x in sn:
	if x in sd:
	    d = sd.replace(x, "")
	    n = sn.replace(x, "")
	    if d == "" or n == "" or d == "0":
		return False
	    d = int(d)
	    n = int(n)
	    if float(n)/d == float(numer)/denom:
		return True
	    else:
		return False


curious_frac = []

for numer in xrange(10, 100):
    for denom in xrange(10, 100):
	if is_cf(numer, denom):
	    curious_frac.append((numer, denom))

print curious_frac


