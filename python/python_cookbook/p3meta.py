#!/usr/bin/python3

from unittest import TestCase, main

def do_decorator():
    from functools import wraps

    def debug(func):
        msg = func.__qualname__
        @wraps(func)
        def wrapper(*args, **kwargs):
            print(msg)
            return func(*args, **kwargs)
        return wrapper

    @debug
    def add(a, b):
        return a + b

    print(add.__dict__)
    print(add(5, 10))

def do_logging():
    from functools import wraps
    import logging

    def debug(func):
        msg = func.__qualname__
        logger = logging.getLogger(func.__module__)
        #logger.setLevel(logging.DEBUG)
        print(logger.level)
        @wraps(func)
        def wrapper(*args, **kwargs):
            logger.info(msg)
            #logger.(msg)
            logger.critical(msg)
            return func(*args, **kwargs)
        return wrapper

    @debug
    def add(a, b):
        return a + b

    add(1, 2)

def do_test0():
    def f0(a, b, ok=0):
        print(a, b, ok)

    f0(0, 1)
    f0(0, 1, 1)

    def f1(a, b, *, ok=0):
        print(a, b, ok)

    f1(0, 1)
    f1(0, 1, ok=1)

def do_test1():
    def closure0(a, b):
        def add():
            print(a+b)
        return add

    f0 = closure0(1, 2)
    f0()
    f0()
    f0()

def do_test2():
    class Foo():
        def __init__(self):
            self.items = [1, 2, 3, 4, 5]
        def __getitem__(self, index):
            if index > len(self.items):
                raise IndexError(index, "> ", len(self.items))
            return self.items[index]
        def __setitem__(self, index, value):
            if index > len(self.items):
                raise IndexError(index, "> ", len(self.items))
            self.items[index] = value
        def __delitem__(self, index):
            del self.items[index]
        def __contains__(self, value):
            if value in self.items:
                return True
            return False
        def __add__(self, inst):
            self.items += inst.items
            return self

    f0 = Foo() 
    f1 = Foo()
    print(f0[1])
    if 10 in f0:
        print("yes")
    else:
        print("no");

    f0[1] = 10
    print(f0[1])
    if 10 in f0:
        print("yes")
    else:
        print("no");

    print(f0.items)
    del f0[1]
    print(f0.items)

    f0 += f1
    print(f0.items)

def do_class_dict():
    class Foo():
        def __init__(self):
            self.x = 1
            self.y = 2
        def f0(self):
            pass

    f0 = Foo()
    print(f0.__dict__)
    print(Foo.__dict__)

    d0 = {}
    d0['x'] = 1
    d0['y'] = 2
    print(d0)

    print(d0['x'])

    f1 = Foo()
    print(f1)
    print(Foo.__dict__)

def do_deco0():
    from functools import wraps

    def debug(func):
        msg = func.__qualname__
        @wraps(func)
        def wrapper(*args, **kwargs):
            print(msg)
            return func(*args, **kwargs)
        return wrapper

    @debug
    def add(a, b):
        ''' doc of add'''
        return a+b

    print(add.__doc__)
    add(1, 2)

class P3Meta(TestCase):
    def test_test(self):
        do_deco0()
        print('Test Done'.center(50, '-'))

if __name__ == '__main__':
    main()
