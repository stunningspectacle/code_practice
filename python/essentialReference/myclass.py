#!/usr/bin/python

class Parent(object):
	obj_num = 0
	age = 10
	def __init__(self, name):
		self.name = name
		Parent.obj_num += 1
		print "this is init of", self.name
	def __del__(self):
		Parent.obj_num -= 1
		print "this is del of", self.name

	def get_name(self):
		print self.name
	def set_name(self, name):
		self.name = name
	def get_age(self):
		print "age of", self.name, "is", self.age
	def set_age(self, age):
		self.age = age
		print "age of", self.name, "is", self.age

def mytest():
	a = Parent("John")
	b = Parent("Bob")
	c = Parent("Jack")
	d = Parent("Mick")

	print Parent.obj_num
	print Parent.age

	a.get_age()
	a.set_age(100)
	b.get_age()
	c.get_age()
	d.get_age()
	print Parent.age

	print a.__dict__
	print b.__dict__
	print c.__dict__
	print d.__dict__
	print Parent.__dict__

if __name__ == "__main__":
	mytest()
else:
	print "I'm imported, so do nothing"
