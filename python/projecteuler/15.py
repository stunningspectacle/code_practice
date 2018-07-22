#!/usr/bin/python


def get_nroute(matrix, width):
    left, top = 0, 0
    for col in xrange(width):
	for row in xrange(width):
	    if col == 0 and row == 0:continue
	    if (row - 1) >= 0:
		left = matrix[col][row - 1]
	    else:
		left = 0
	    if (col - 1) >= 0:
		top = matrix[col - 1][row]
	    else:
		top = 0
	    matrix[col][row] = left + top
    return matrix

def main(num):
    matrix = [[0] * num for i in xrange(num)]
    matrix[0][0] = 1
    matrix = get_nroute(matrix, num)
    return matrix

print main(21)
