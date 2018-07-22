#!/usr/bin/python

result = []
for i in xrange(10, 2540161):
    if i % 10000 == 0:
	print "Now in %d of 254" % (i // 10000)
    s = str(i)
    add = sum(reduce(lambda x, y: x*y, range(1, int(c) + 1))
	      for c in s if c != "0")
    add += s.count("0")
    if i == add:
	print i,
	result.append(i)	

print result
print len(result)
print sum(result)
	
