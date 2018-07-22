#!/usr/bin/python
import time

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

def n_proper_coins(coins_list):
    nums = len(coins_list)


start = time.time()

combines = []
pences = [1, 2, 5, 10, 20, 50, 100]

comb = get_combine(pences, 1)
for x in comb:
    for a in xrange((200 // x) + 1):
	if x * a == 200:
	    combines.append((x, a))
print len(combines)


end = time.time()

print "\nConsume %f seconds" % (end - start)


