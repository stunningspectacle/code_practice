#!/usr/bin/python

first = 1
second = 1

count = 2
while True:
    curr = first + second
    count += 1
    if len(str(curr)) >= 1000:
	print count, curr
	print len(str(curr))
	break
    first = second
    second = curr
