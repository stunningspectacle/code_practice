#!/usr/bin/python

import struct

n = "16"

f = open("abc.bin", "w+")

c = struct.pack("s", n)
print c
f.close()
