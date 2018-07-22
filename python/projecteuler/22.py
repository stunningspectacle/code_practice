#!/usr/bin/python

import string

text = open("names.txt", "r").read()
text = text.replace("\"", "")
text = text.split(",")
text.sort()

alph_dict = {}
for i, letter in enumerate(string.ascii_uppercase):
    alph_dict[letter] = i + 1

score = 0
for i, name in enumerate(text):
    score += sum(alph_dict[c] for c in name) * (i + 1)
print score

