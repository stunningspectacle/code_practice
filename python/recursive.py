#!/usr/bin/python
import sys

def factorial(num):
    if num == 1:
	return 1;
    return factorial(num - 1) * num

num = int(sys.argv[1])

print factorial(num)
