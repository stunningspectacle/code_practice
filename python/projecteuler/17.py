#!/usr/bin/python

num_dict = {1:	    "one",
	    2:	    "two",
	    3:	    "three",
	    4:	    "four",
	    5:	    "five",
	    6:	    "six",
	    7:	    "seven",
	    8:	    "eight",
	    9:	    "nine",
	    10:	    "ten",
	    11:	    "eleven",
	    12:	    "twelve",
	    13:	    "thirteen",
	    14:	    "fourteen",
	    15:	    "fifteen",
	    16:	    "sixteen",
	    17:	    "seventeen",
	    18:	    "eighteen",
	    19:	    "nineteen",
	    20:	    "twenty",
	    30:	    "thirty",
	    40:	    "forty",
	    50:	    "fifty",
	    60:	    "sixty",
	    70:	    "seventy",
	    80:	    "eighty",
	    90:	    "ninety",
	    100:    "hundred",
	    1000:   "thousand"
}

def get_tens(num):
    i = num % 100
    if i == 0:
	return ""
    if i <= 20 or i % 10 == 0:
	return num_dict[i]
    units = i % 10
    tens = i - units
    return (num_dict[tens] + num_dict[units])

def get_hundreds(num):
    hundreds = num // 100
    if hundreds == 0:
	return ""
    else:
	if num > 100 and num % 100 != 0:
	    return (num_dict[hundreds] + num_dict[100] + "and")
	else:
	    return (num_dict[hundreds] + num_dict[100])


def get_nletters(num):
    return (get_hundreds(num) + get_tens(num))

count = 0
for i in xrange(1, 1000):
    count += len(get_nletters(i))

print count

