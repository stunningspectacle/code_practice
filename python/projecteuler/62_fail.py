#!/usr/bin/python

import euler

TOP = 10000
BOTTOM = 11

def cube_perms(cube_list, mark_list, num, tail=""):
    perms = euler.xperm(list(str(num)), len(str(num)))
    results = [int(str(num) + tail)]
    tmp = [int(str(num) + tail)]
    for perm in perms:
	if perm[0] == "0":
	    continue
	new_num = int("".join(perm)+tail)
	if new_num in tmp:
	    continue
	tmp.append(new_num)
	for i in xrange(len(cube_list)):
	    if cube_list[i] < new_num:
		continue
	    if cube_list[i] == new_num:
		if new_num not in results:
		    results.append(new_num)
		    mark_list[i] = 1
	    break
    return (mark_list, results)

def prob62():
    cubes = [i ** 3 for i in xrange(BOTTOM, TOP)]
    mark = [0] * len(cubes)
    for i,num in enumerate(cubes):
	if i % 50 == 0:
	    print "i=", i + BOTTOM, num

	if (mark[i] == 1 or 
	    str(num)[-2:] != "25"):
	    continue
	mark0, results = cube_perms(cubes, mark, int(str(num)[:-2]), "25")
	if len(results) > 1:
	    mark, results = cube_perms(cubes, mark, num)
	    results.sort()
	    print results


prob62()
    

