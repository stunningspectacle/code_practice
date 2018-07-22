#!/usr/bin/python

def get_dn(num):
    start = 1
    width = 0
    while True:
	width = len(str(start))
	n_digits = (10 ** width) - start
	rang = width * n_digits
	if num <= rang:
	    break
	num -= rang
	start = start * 10

    div, mod = divmod(num, width)
    if mod == 0:
	return str(start + div - 1)[-1]
    return str(start + div)[mod - 1]


result = 1
for i in xrange(7):
    result = result * int(get_dn(10 ** i))

print result
	
    
