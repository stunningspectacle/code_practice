#!/usr/bin/python

def use_dict():
	mydict = {
			"google": 1000,
			"intel":  50,
			"microsoft": 100,
			}
	print type(mydict)
	print mydict

	d0 = dict.fromkeys(mydict, 0)
	print d0
	d1 = dict.fromkeys([i for i in range(10)])
	print d1

def use_set():
	set0 = set("what do you think is right?")
	print len(set0)

class Foo(object):
	pass

def test_types():
	print use_set.__doc__
	print use_set.__code__
	print type(use_set)
	print type(len)
	print type(object)

	print type(Foo)
	print type(object)
	f = Foo()
	print type(f)
	print type
	print type(1)

	list0 = [1, 2, 3, 4]
	print list0, type(list0)
	a = str(list0)
	b = repr(list0)
	print type(a), [c for c in a]
	print type(b), [c for c in b]
	d = eval(a)
	print type(d), [c for c in d]

