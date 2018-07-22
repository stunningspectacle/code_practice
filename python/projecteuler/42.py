#!/usr/bin/python
import math
def is_triang(num):
    num = 2 * num
    sr = int(math.sqrt(num))
    if num == (sr * (sr + 1)):
	return True
    return False

print is_triang(21)

text = open("words.txt", "r").read()
text = text.replace("\"", "").split(",")

count = 0

for name in text:
    if is_triang(sum((ord(c) - ord("A") + 1) for c in name)):
	count += 1

print count
