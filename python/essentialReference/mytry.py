#!/usr/bin/python

import sys
import myclass

f = open("abc.txt", "rw")
while True:
	line = f.readline()
	if not line:
		break
	print line

print sys.path
