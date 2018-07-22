#!/usr/bin/python

import euler
import time

t = time.clock()

i = 4
TOP = 1000000

while True:
    m = reduce(lambda x, y: x*y, euler.primes_beforex(i))
    if m > TOP: break
    print m
    i += 2

print time.clock() - t
