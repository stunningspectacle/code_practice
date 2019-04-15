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

def do_readinto():
    import os.path

    size = os.path.getsize('sample.txt')
    print(size)
    s = bytearray(size)
    with open('sample.txt', 'rb') as f: # must open as 'b'
        f.readinto(s)
    print(s)

    s0 = memoryview(s)
    s1 = s0[-5:]
    print(s1)

    s1[:] = b'KKKKK'
    print(s)

def do_mmap():
    import os
    import mmap

    size = 100
    s = b'ABCKKK'
    slen = len(s)
    fn = 'mmap.bin'
    with open(fn, 'wb') as f:
        f.seek(size - len(s))
        f.write(s)
    print(os.path.getsize(fn))

    fd = os.open(fn, os.O_RDWR) # need to open by os.open to get fd
    with mmap.mmap(fd, size, access=mmap.ACCESS_WRITE) as arr:
        print(len(arr))
        print(arr[-slen:])

        ns = b'MYJAC'
        ns_len = len(ns)
        arr[-ns_len:] = ns
        print(arr[-ns_len:])

def do_os_path():
    import os

    path = '~/study/code_practice/python/python_cookbook/'
    print("expanduser:", os.path.expanduser(path))
    print("dirname:", os.path.dirname(path))
    print("basename:", os.path.basename(path))

def do_os_listdir():
    import os

    mydir = '~/study/code_practice/python'
    mydir = os.path.expanduser(mydir)
    names = os.listdir(mydir)
    files = [os.path.join(mydir, name) for name in names
                if os.path.isfile(os.path.join(mydir, name))]
    dirs = [os.path.join(mydir, name) for name in names
                if os.path.isdir(os.path.join(mydir, name))]
    print(files)
    print(dirs)

    import glob
    pyfiles = glob.glob(os.path.join(mydir, '*.py'))
    print("py files:".rjust(30, '*'), pyfiles)

    import fnmatch
    pycfiles = [os.path.join(mydir, name) for name in os.listdir(mydir)
            if fnmatch.fnmatch(name, "*.pyc")]
    print("pyc files:".rjust(30, '*'), pycfiles)

    for fn in pycfiles:
        print(os.stat(fn))
        mtime = os.path.getmtime(fn)
        print(mtime)

        import time
        lt = time.localtime(mtime)
        print(lt)
        asct = time.asctime(lt)
        print(asct)

def do_unicode_file():
    import os

    fn = '你好.txt'

    with open(fn, 'w') as f:
        f.write('好的')

    names = os.listdir(b'.') # use b'.' to convert unicode fn into byte string
    for name in names:
        print(name)
    print(names)

    with open(b'\xe4\xbd\xa0\xe5\xa5\xbd.txt', 'r') as f:
        print(f.read())

def do_file_detach_3layers():
    import io

    fn = 'sample.txt'
    f = open(fn, 'w')
    print('f:', f)
    print('f.buffer:', f.buffer)
    print('f.buffer.raw:', f.buffer.raw)

    f0 = f.detach()
    print('f0:', f0)

    f1 = io.TextIOWrapper(f0, encoding='latin-1')
    print('f1:', f1)

def do_read_binary_into_textfile():
    fn = 'sample0.txt'

    with open(fn, 'at') as f:
        f.buffer.write(b'FFABC\n') # write to file's binary buffer directly

def do_tempfile():
    import tempfile

    with tempfile.TemporaryFile('w+t') as f:
        print(f)
        print(f.name)
        f.write('testing')
        f.seek(0)
        print(f.readline())

    print("temp folder:", tempfile.gettempdir())
    with tempfile.NamedTemporaryFile('w+t') as f:
        print(f)
        print(f.name)

    with tempfile.TemporaryDirectory() as dirname:
        print('dir is:', dirname)

def do_serialize_obj():
    import os
    import pickle

    fn = 'serialize_obj.pickle'
    if not os.path.exists(fn):
        with open(fn, 'xb') as f:
            pass

    with open(fn, 'wb') as f:
        print(f)
        pickle.dump([1, 2, 3, 4, 5], f)
        pickle.dump({"leo": 100, "john": 10, "mike": 60}, f)
        pickle.dump("this is good", f)

    with open(fn, 'rb') as f:
        print(pickle.load(f))
        print(pickle.load(f))
        print(pickle.load(f))

    s0 = pickle.dumps([1, 2, 3, 4, 5])
    s1 = pickle.dumps({"leo": 100, "john": 10, "mike": 60})
    print(s0)
    print(s1)

    obj0 = pickle.loads(s0)
    obj1 = pickle.loads(s1)
    print(obj0)
    print(obj1)

    print(' use json: '.center(25, '#'))
    import json
    fn = 'serialize_obj.json'
    if not os.path.exists(fn):
        with open(fn, 'xt') as f:
            pass

    ''' jason can only dump/load one object, so need to combine all the
    record into one obj'''
    with open(fn, 'w') as f:
        print(f)
        save = []
        save.append([1, 2, 3, 4, 5])
        save.append({"leo": 100, "john": 10, "mike": 60})
        save.append("this is good")
        json.dump(save, f)

    with open(fn, 'r') as f:
        print(f)
        save = json.load(f)
        for obj in save:
            print(obj)

    s0 = json.dumps([1, 2, 3, 4, 5])
    s1 = json.dumps({"leo": 100, "john": 10, "mike": 60})
    print(s0)
    print(s1)

    obj0 = json.loads(s0)
    obj1 = json.loads(s1)
    print(obj0)
    print(obj1)

class C05TestCase(TestCase):
    def test_test(self):
        #s = input("input something:")
        do_serialize_obj()
        print("Test Done".center(80, '-'))

if __name__ == '__main__':
    main()
