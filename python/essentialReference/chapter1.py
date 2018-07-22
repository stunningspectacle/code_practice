#!/usr/bin/python
import sys
import time
import threading

def code1_1():
	principle = 100
	rate = 0.05
	years = 15
	count = 1

	while count <= years:
		principle *= (1 + rate)
		print "%2d, %0.5f" % (count, principle)
		count += 1

def code1_2():
	if len(sys.argv) != 2:
		print "please input a file name"
		raise SystemExit(1)

	f = open(sys.argv[1])
	lines = f.readlines()
	f.close()

	print lines

	values = [float(value) for value in lines]

	print "the max is", max(values)
	print "the min is", min(values)

def test1():
	names = ['jack', 'john', 'leo', 'susi', 'mack', 'mike']
	print names
	n1 = names[:3]
	print n1
	n1[0] = 'changed'
	print n1
	print names
	names.append("mimo")
	print names
	names.insert(0, "myfirst")
	print names

def test2():
	s = open("float.txt").read()
	print s[1]

def test_set():
	t0 = set([10, 0, 0, 9, 8, 5, 100, 10293])
	print t0
	t1 = set("Heloolll")
	print t1
	t1.add('k')
	print t1
	t1.update(c for c in "kajskfja;sdjfaf")
	print t1
	t1.update("5918398102384")
	print t1
	for c in "0123456789":
		if c in t1:
			t1.remove(c)
	print t1

def test_dict():
	d0 = dict()
	d1 = { }
	d2 = {"google": 100,
			"intel": 20,
			"amazon": 80,
			"micosoft": 70
			}
	for key in d2:
		#print type(key), type(d2[key])
		print format(key, "10s"),  format(d2[key], "5d")

	print d0, d1
	print d2

	if "intel" in d2:
		p = d2["intel"]
	else:
		p = 0.0
	p1 = d2.get("google", 0.0)
	print p1

	p2 = d2.get("ebay", 0.0)
	print p2
	keys = list(d2)
	print keys
	del d2["intel"]
	print d2

def my_generator(n):
	print "start counting down!"
	while n > 0:
		yield n
		n -= 1

def test_gen():
	c = my_generator(10)
	v = [i for i in c]
	v1 =  [i for i in my_generator(50)]

	print v
	print v1

def tail(f):
	while True:
		line = f.readline()
		if not line:
			time.sleep(1)
			continue
		yield line

def grep(text, search):
	for line in text:
		if search in line:
			yield line

def use_tail():
	newlines = tail(open("chapter1.py"))
	search = grep(newlines, "print")
	for line in search:
		print line,

def print_matches(match_text):
	print "Looking for", match_text
	while True:
		line = (yield)
		if match_text in line:
			print line,


def use_coroutine():
	coroutine = print_matches("print")
	coroutine.next()
	coroutine.send("aaaaaabbbbbb")
	time.sleep(1)
	coroutine.send("aaaaaaprintaaaaa")
	time.sleep(1)
	coroutine.send("ccccccprintccccc")
	time.sleep(1)
	coroutine.close()

	matchers = [
			print_matches("print"),
			print_matches("aaaa"),
			print_matches("bbbb")
			]
	for m in matchers:
		m.next()
	for line in tail(open("chapter1.py")):
		for m in matchers:
			m.send(line)

def test_exception():
	try:
		f = open("abc.txt")
		f.close()
	except IOError as e:
		print e
	print "kkk"

def test_runtime_error():
	print "I want to raise an error"
	try:
		raise RuntimeError("just a test")
	except RuntimeError as e:
		print e

	print "after runtime error"

def test_with():
	lock = threading.Lock()
	with lock:
		print "just print with lock"
	print "now lock is released"


def use_zip():
	line = "GOOG, 100, 490.10"
	field_type = [str, int, float]
	raw_field = line.split(',')
	print raw_field
	print zip(field_type, raw_field)
	field = [ty(val) for ty, val in zip(field_type, raw_field)]
	print field


def test_copy():
	a = [i for i in range(0, 10)]
	b = list(a)
	c = a
	print "a:", a
	print "b:", b
	print "c:", c
	b[0] = 100
	print "a:", a
	print "b:", b
	print "c:", c


def test_slice():
	a = [i for i in range(0, 10)]
	b = a[0:8:2]
	print b
	print len(b)

test_slice()
