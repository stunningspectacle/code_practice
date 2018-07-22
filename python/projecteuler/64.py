#!/usr/bin/python

def sr_int(num):
    i = 1
    while True:
	if i ** 2 > num:
	    break
	if i ** 2 == num:
	    return 0
	i += 1
    return i - 1


def split_num(sr, int_minus, denom, sr_int):
    period = []
    repeat = []
    while True:
	if (int_minus, denom) in repeat:
	    return period
	repeat.append((int_minus, denom))
	new_denom = sr - int_minus* int_minus
	assert new_denom % denom == 0, "%s %s != 0" % (str(new_denom), str(denom))
	denom = new_denom / denom
	a = (sr_int + int_minus) // denom
	period.append(a)
	b = (sr_int + int_minus) % denom
#	assert int_minus > b, "int_minus <= b"
	int_minus = sr_int - b

    return period


def prob64():
    count = 0
    maxx = (0, 0)
    for i in xrange(2, 1001):
	sr = sr_int(i)
	if sr == 0: continue
	period = split_num(i, sr, 1, sr)
	print i, period, len(period)
	if len(period) % 2 != 0:
	    count += 1
	if sum(period) > maxx[0]:
	    maxx = (sum(period), i)
    print "count=", count
    print maxx

prob64()




