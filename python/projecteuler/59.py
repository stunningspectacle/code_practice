#!/usr/bin/python
import string
import euler

keys = [ord(c) for c in string.ascii_lowercase]

def statis(text):
    statis = [0] * 256
    for c in text:
	statis[ord(c)] += 1
    for i in xrange(len(statis)):
	if statis[i] != 0:
	    print chr(i), statis[i], float((statis)[i])/len(text)


def test():
    text = open("cipher1.txt", "r").read().strip().split(",")
    ks = []
    for key in keys:
	mark = 0
	for item in text[2::3]:
	    ch = chr(key ^ int(item))
	    if ch not in string.printable:
		mark = 1
	if mark == 0:
	    ks.append(key)
	    print chr(key)
	else:
	    print "no"
    
    for key in ks:
	print "######", chr(key), "#####"
	for item in text[2::3]:
	    ch = chr(key ^ int(item))
	    print ch,
	print "\n"

def prob59():
    text = open("cipher1.txt", "r").read().strip().split(",")
    keys = "god"
    plain = []
    i = 0
    for item in text:
	ch = chr(int(item) ^ ord(keys[i]))
	plain.append(ch)
	i = (i + 1) % len(keys)
    print sum(ord(c) for c in plain)
    print "".join(plain)

    #statis("".join(plain))
    #statis("".join(chr(int(item)) for item in text))

prob59()
	

    



