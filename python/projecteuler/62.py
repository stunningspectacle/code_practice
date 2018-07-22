#!/usr/bin/python

def sort_cube(num):
    cube = list(str(num ** 3))
    cube.sort()
    return "".join(cube)

def prob62():
    TOP = 10000
    cube = {}
    for i in xrange(11, TOP):
	str_cube = sort_cube(i)
	if str_cube in cube:
	    cube[str_cube] += [i]
	    if len(cube[str_cube]) == 5:
		print cube[str_cube]
		print [c ** 3 for c in cube[str_cube]]
	else:
	    cube[str_cube] = [i]


prob62()

    
