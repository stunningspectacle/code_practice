#!/usr/bin/python3

from unittest import TestCase, main

def do_iter():
    items = [1, 2, 3, 4]
    it = iter(items)
    print(it)

    try:
        while True:
            a = next(it)
            print(a)
    except StopIteration:
        pass

def do_reversed():
    class CountDown:
        def __init__(self, start):
            self.start = start

        def __iter__(self):
            start = self.start
            while start > 0:
                yield start
                start -= 1
        
        def __reversed__(self):
            i = 1
            while i <= self.start:
                yield i
                i += 1

    for i in CountDown(5):
        print(i)
    for i in reversed(CountDown(5)):
        print(i)

def do_enumerate():
    from collections import defaultdict

    simple_dict = defaultdict(list)
    with open("sample.txt") as f:
        for index, line in enumerate(f, 1):
            words = [w.strip().lower() for w in line.split()]
            for word in words:
                simple_dict[word].append(index)
    print(simple_dict)

def do_zip():
    headers = ['name', 'shares', 'price']
    values = ['ACME', 100, 490.1]

    s = zip(headers, values)
    print(list(s))

    s = dict(zip(headers, values))
    print(s)

def do_pipe():
    import os

    def gen_filenames():
        for path, dirlist, filelist in os.walk("/home/leo/study/code_practice/python"):
            for f in filelist:
                if f.endswith(".sh"):
                    yield os.path.join(path, f)

    def gen_opener(filenames):
        for filename in filenames:
            f = open(filename)
            yield f
            f.close()

    def gen_counter(opened_files):
        for f in opened_files:
            for index, line in enumerate(f):
                #print(index, line)
                yield len(line.strip().split())
    
    filenames = gen_filenames()
    opener = gen_opener(filenames)

    print(sum(gen_counter(opener)))

def do_flatten():
    from collections import Iterable

    items = [1, 2, [3, 4, [5, 6], 7], 8]
    def flatten(items, ignore_types=(str, bytes)):
        for i in items:
            if isinstance(i, Iterable) and not isinstance(i, ignore_types):
                yield from flatten(i)
            else:
                yield i
    
    print(list(flatten(items)))

class C03TestCase(TestCase):
    def test_test(self):
        do_flatten()
        print("---------Test Done--------")

if __name__ == '__main__':
    main()



