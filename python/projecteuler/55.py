#!/usr/bin/python

def is_lychrel(num):
    count = 50
    while count > 0:
	num = num + int(str(num)[::-1])
	if str(num) == str(num)[::-1]:
	    return False
	count -= 1
    return True


def prob51():
    palin = []
    for i in xrange(1, 10000):
	if is_lychrel(i):
	    palin.append(i)
    
    print palin
    print len(palin)


prob51()

	    


