#!/usr/bin/python

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


test0()
