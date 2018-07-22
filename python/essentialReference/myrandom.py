#!/usr/bin/python

import random

times = 1000000
mark = [0] * 100
for i in range(times):
	mark[random.randint(0, 99)] += 1

print mark
print min(mark)
print max(mark)


