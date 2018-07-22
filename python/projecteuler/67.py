#!/usr/bin/python
test0 = """\
3
7 5
2 4 6
8 5 9 3"""

test1 = """\
75
95 64
17 47 82
18 35 87 10
20 04 82 47 65
19 01 23 75 03 34
88 02 77 73 07 63 67
99 65 04 28 06 16 70 92
41 41 26 56 83 40 80 70 33
41 48 72 33 47 32 37 16 94 29
53 71 44 65 25 43 91 52 97 51 14
70 11 33 28 77 73 17 78 39 68 17 57
91 71 52 38 17 14 91 43 58 50 27 29 48
63 66 04 68 89 53 67 30 73 16 69 87 40 31
04 62 98 27 23 09 70 98 73 93 38 53 60 04 23"""

def get_path(triangle):
    parent = len(triangle) - 2
    while True:
	if parent < 0: break
	len_parent = len(triangle[parent])
	for x in xrange(len_parent):
	    left_child = triangle[parent + 1][x]
	    right_child = triangle[parent + 1][x + 1]
	    triangle[parent][x] += (left_child > right_child and left_child) or right_child
	parent -= 1

    return triangle

def prob67():
    text = open("triangle.txt", "r").read().split("\n")
    triangle = []
    for line in text[:-1]:
	triangle.append([int(s) for s in line.split(" ")])
    path = get_path(triangle) 
    print path[0]


prob67()


