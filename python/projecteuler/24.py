#!/usr/bin/python

factorials = [reduce(lambda x, y: x * y,
		    range(1, i + 1))
		    for i in xrange(1, 11)]
print factorials
factorials.sort(reverse=True)

num = 1000000
#num = 2
print "num=", num
num = num - 1

orig = range(10)

result = ""
for i, x in enumerate(factorials):
    val = num // x
    if val != 0:
	print 10 - i, ":", num // x
	num = num % x
	result = result + str(orig[val])
	del orig[val]
    else:
	print 10 - i, "None"

result = result + "".join(str(x) for x in orig)

print result
	    
