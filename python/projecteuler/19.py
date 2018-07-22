#!/usr/bin/python

import calendar


count = 0
for year in xrange(1901, 2001):
    for month in xrange(1, 13):
	weekday, ndays = calendar.monthrange(year, month)
	if weekday == 6:
	    count += 1
	    print year, month, count

