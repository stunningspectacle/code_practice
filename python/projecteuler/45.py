#!/usr/bin/python

import math
import sys

def is_triangle(num):
    tmp = 2 * num
    sr = math.sqrt(4 * tmp + 1)
    if sr != int(sr):
	return False
    if (sr - 1) % 2 == 0:
	return True
    return False

def is_panta(num):
    tmp = 2 * num
    sr = math.sqrt(12 * tmp + 1)
    if sr != int(sr):
	return False
    if (sr + 1) % 6 == 0:
	return True 
    return False

def is_hex(num):
    sr = math.sqrt(8 * num + 1)
    if sr != int(sr):
	return False
    if (sr + 1) % 4 == 0:
	return True
    return False

import time

t = time.clock()

nums = []
i = 1
while True:
    if i % 10000 == 0:
	print "i=", i
    n = 2 * i * i - i
    if (is_triangle(n) and is_panta(n)):
	print n
	nums.append(n)
	if len(nums) == 3:
	    print nums
	    print "time consumed:", time.clock() - t
	    sys.exit(0)
    i += 1



    
