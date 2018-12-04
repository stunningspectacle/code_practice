#!/usr/bin/python
import sys

MAX_ORDER = 10

def usage():
    print "Usage:", sys.argv[0], "num"
    exit(0)

if len(sys.argv) != 2:
    usage()

try:
    num = int(sys.argv[1])
except Exception as e:
    print e
    usage()

order = 0
while order < MAX_ORDER:
    buddy = (num ^ (1 << order))
    combined_num = num & (~(1 << order))
    print "For order", order, ", %d's buddy is %d," % (num, buddy), "new index is %d" % combined_num
    num = combined_num
    order += 1

