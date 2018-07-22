#!/usr/bin/python

import sys

def test0():
	s = "okmyspam"

	if "spam" in s:
		print "yes"
	else:
		print "no"


	f = open("1_1.py")
	line = f.readline()

	while line:
		print line,
		line = f.readline()
	f.close()
	print "##########################"


	f = open("test.py")
	for line in f:
		print line,

	f.close()
	print "aaaaaaaaaaaaaaaaaaaaaaaaaa"

	for line in open("test.py"):
		print line,

	print "bbbbbbbbbbbbbbbbbbbbbbbbbb"


	f = open("try.py", "w")
	for line in open("test.py"):
		print >>f, line,
	f.close()

	f = open("try1.py", "w")
	for line in open("test.py"):
		f.write(line)
	f.close()

	line = raw_input("please input a line:")
	print "you input", line

def test1():
	sys.stdout.write("please input:")
	line = sys.stdin.readline()
	sys.stdout.write(line)

	s = ''' OK this is line 1
		this is line 2
		<tag>abc</tag>'''
	print s

x = 10.01

s0 = "the float is " + str(x)
print s0
s1 = "this float is " + repr(x)
print s1
s2 = "this float is " + format(x, "0.5f")
print s2
