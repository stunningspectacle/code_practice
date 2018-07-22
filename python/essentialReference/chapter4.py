#!/usr/bin/python

def test_list():
	a0 = [i for i in range(5)]
	b0 = a0 * 2
	c0 = [a0]
	d0 = c0 * 2
	print a0
	print b0, type(b0)
	print d0, type(d0)

def test():
	a0 = [i for i in range(5)]
	b0 = a0
	print a0
	del a0[2:4]
	print a0, b0

def format_dict_to_str():
	stock = {
			"name": "GOOGLE",
			"shares": 100,
			"price": 500.2,
			}
	print stock
	s = "%(shares)d %(name)s shares at %(price)0.2f$" % stock
	print s

stock = {
		"name": "GOOGLE",
		"shares": 100,
		"price": 500.2,
		}
	
def test_vars():
	a = "ok"
	b = [1, 2, 3]
	name = "jack"
	print vars()

def advanced_str_format():
	s = "{0} is {{ }} {1} not {name}".format("jack", "julie", name = "func")
	print s

	s = "{0[shares]} {0[name]} stocks at {0[price]}$".format(stock)
	print s

def test_dict1():
	d = { }
	d[1, 2, 3] = "fish"
	print len(d)
	print d

def repr_and_eval():
	s = repr([1, 2, 3, 4])
	print s, type(s)
	list0 = eval(s)
	print list0, type(list0)

def list0_in():
	a0 = [i if i % 2 == 0 else 100 for i in range(100)]
	#b0 = [i for i in range(50) if i % 2 == 0 else 100]
	b0 = [i if i % 2 == 0 else 99 for i in xrange(50)]
	print  a0
	print b0

def test_with():
	try:
		with open("abcd.txt", "r") as f:
			line = f.readline()
		f.close()
	except Exception as e:
		print e
	print "this is end"


def test_with_again():
	try:
		with open("abc.txt", "r") as f:
			while True:
				line = f.readline()
				if not line:
					break
				print line,
	except Exception as e:
		print e


test_with_again()

