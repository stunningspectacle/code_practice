#!/usr/bin/python

class Account(object):
	num_accounts = 0
	def __init__(self, name, balance):
		self.name = name
		self.balance = balance
		self.num_accounts += 1
	def __del__(self):
		self.num_accounts -= 1
	def deposit(self, amt):
		self.balance += amt
	def withdraw(self, amt):
		self.balance -= amt
	def inquiry(self):
		return self.balance



a = Account("Bob", 100)
b = Account("Leo", 1000)

print a.inquiry()
