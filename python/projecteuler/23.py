#!/usr/bin/python
import time

def get_prime_factors(num):
    root = num
    prime_factors = []
    i = 1
    while True:
	i += 1
	if num % i == 0:
	    num = num / i
	    prime_factors.append(i)
	    i = 1
	if num == 1: break
    
    #factors = []
    #for i in prime_factors:
    #    if i not in factors:
    #        factors.append(i)
    #    else:
    #        tmp = i * i
    #        while True:
    #    	if tmp not in factors:
    #    	    factors.append(tmp)
    #    	    break
    #    	tmp = tmp * i
    #factors.sort()
    return prime_factors

def get_combine(list, num):
    mark = [1] * num
    length = len(list)
    mark.extend([0] * (length - num))
    combine_list = []
    while True:
	combine_list.append([list[i] for i in xrange(len(mark)) if mark[i] == 1])
	try:
	    for i in xrange(length):
		if mark[i] == 1 and mark[i + 1] == 0:
		    mark[i] = 0 
		    mark[i + 1] = 1
		    count_1 = 0
		    for j in xrange(i+1):
			if mark[j] == 1:
			    count_1 += 1
			    mark[j] = 0
		    for j in xrange(count_1):
			mark[j] = 1
		    changed = 1
		    break
	except:
	    break
    return combine_list

def get_factors(num):
    prime_factors = get_prime_factors(num)
    length = len(prime_factors)
    factors = []
    for i in xrange(1, length + 1):
	combine_list = get_combine(prime_factors, i)
	tmp_factors = [reduce(lambda x, y: x*y, combine) 
			for combine in combine_list]
	for x in tmp_factors:
	    if x not in factors and x < num:
		factors.append(x)
    factors.sort()
    factors.insert(0, 1)
    return factors 

start = time.time()

NUMBER = 28122
abundant_nums = []
for i in xrange(1, NUMBER+1):
    prop_fact = get_factors(i)
    if sum(prop_fact) > i:
	abundant_nums.append(i)
	print i,
total = len(abundant_nums)

two_abun_add = []
mark = [0] * (NUMBER + 1)

for i, x in enumerate(abundant_nums):
    print "index=%d of total %d" % (i+1, total)
    for y in abundant_nums[i:]:
	num = x + y
	if num > NUMBER: break
	if mark[num] == 0:
	    two_abun_add.append(num)
	    mark[num] = 1

print "Done"
print sum(range(1, NUMBER+1))
print sum(range(1, NUMBER+1)) - sum(two_abun_add)
two_abun_add.sort()
print two_abun_add[:50]

end = time.time()

print "\nConsume %f seconds" % (end - start)


#########if for i = di, all i will meet i = 2**(x-1)*(2**x-1)

