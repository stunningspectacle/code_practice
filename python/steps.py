#!/usr/bin/python

def get_steps(num):
    if num < 1:
	return 0
    steps = [0, 1, 2]
    cnt = 3
    while cnt <= num:
	steps.append(sum(steps) + 1)
	cnt += 1
    print steps[num]

