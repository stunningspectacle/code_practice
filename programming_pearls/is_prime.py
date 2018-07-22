#!/usr/bin/python

def is_prime(n):
	if n == 2:
		return True
	if n % 2 == 0:
		return False
	i = 3
	while i * i <= n:
		if n % i == 0:
			return False
		i += 2
	return True

def is_prime2(n):
	if n == 2:
		return True
	i = 2
	while i * i <= n:
		if n % i == 0:
			return False
		i += 1
	return True

print [i for i in range(2, 1000) if is_prime(i)]

print [i for i in range(2, 1000) if is_prime2(i)]

