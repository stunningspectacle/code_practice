#!/usr/bin/python3

from unittest import TestCase, main

def do_redirect_print():
    import sys

    print("default encoding:", sys.getdefaultencoding())
    with open('mylog.log', 'w') as f:
        print("writing to log1", file=f)
        print("writing to log2", file=f)
        print("done")

    with open('mylog.log', 'r', newline='') as f:
        s = f.read()

def do_print_sep_end():
    print('ACME', 50, 91.5, sep=',')
    print('ACME', 50, 91.5, sep=',', end='!!\n')

    for i in range(10):
        print(i, end=' ')
    print('\n', end='')

    row = ('ACME', 50, 91.5)
    print(row)
    print(*row)
    print(*row, sep=',')

    s = "abcdefg"
    print(s)
    print(*s)
    print(list(s))

    row = {"leo": 10, "john": 11, "jack": 12}
    print(row)
    print(*row)
    print(*row.keys())
    print(*row.values())
    try:
        print(**row) #error
    except:
        pass

def do_binary():
    t = "hello world"
    b = b"hello world"
    r = r"hello world"
    for i, j, k in zip(t, b, r):
        print(i, j, k)

    with open("my.bin", "wb") as f:
        f.write(t.encode("utf-8"))
    with open("my.bin", "rb") as f:
        s0 = f.read(8)
        s = s0.decode("utf-8")
        print(s0)
        print(s)

    import array
    k = [1, 2, 3, 4, 5]
    nums = array.array('i', k) # 'i' means signed integer
    print(nums)
    with open("my0.bin", "wb") as f:
        f.write(nums)
    with open("my0.bin", "rb") as f:
        n0 = f.read()
        n = array.array('i', n0)
        print(n)
    n1 = array.array('i', [0] * 10)
    with open("my0.bin", "rb") as f:
        f.readinto(n1)
        print(n1)

def do_stringIO_bytesIO():
    import io

    s = io.StringIO()
    s.write("test1")
    print("test2", file=s)
    print(s.getvalue())

    s = io.BytesIO()
    s.write(b'test byte 0')
    print(s.getvalue())

def do_gz_bz2():
    f = open('c01_struct_algo.py', 'rb')
    text = f.read()
    f.close()

    import bz2, gzip
    with bz2.open('my.bz2', 'wb', compresslevel=9) as f0:
        with gzip.open('my.gz', 'wb', compresslevel=9) as f1:
            f0.write(text)
            f1.write(text)

    with open('mysample.txt', 'wb') as f:
        with bz2.open(f, 'wb') as b: # Open from f
            b.write(text)

def do_fixed_block():
    from functools import partial

    MYSIZE = 100
    with open('sample.txt', 'rb') as f:
        records = iter(partial(f.read, MYSIZE), b'')
        for r in records:
            print(r)
    
class C05TestCase(TestCase):
    def test_test(self):
        #s = input("input something:")
        do_fixed_block()
        print("Test Done".center(50, '-'))

if __name__ == '__main__':
    main()
