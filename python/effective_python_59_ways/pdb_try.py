#!/usr/bin/python

def test0():
    a, b, c = 2, 3, 5
    import pdb; pdb.set_trace()
    while True:
        a += a
        b += b
        c += c
        if c >= 100:
            break



test0()
print("f 1")
test0()
