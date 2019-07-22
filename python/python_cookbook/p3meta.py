#!/usr/bin/python3

from unittest import TestCase, main
from functools import wraps, partial
import logging

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

def do_log():
    import os
    from functools import wraps
    import logging

    def debug(func):
        if 'DEBUG' not in os.environ:
            return func
        if os.environ['DEBUG'] == '0':
            return func
        msg = func.__qualname__
        log = logging.getLogger()
        @wraps(func)
        def wrapper(*args, **kwargs):
            log.critical(msg)
            return func(*args, **kwargs)
        return wrapper

    @debug
    def add(a, b):
        return a + b

    add(1, 2)

def do_prefix_deco():
    def debug(prefix=''):
        def decorator(func):
            msg = prefix + func.__qualname__
            log = logging.getLogger()
            @wraps(func)
            def wrapper(*args, **kwargs):
                log.critical(msg)
                return func(*args, **kwargs)
            return wrapper
        return decorator

    @debug('+++++++')
    def add(a, b):
        return a + b

    @debug('*******')
    def mul(a, b):
        return a * b

    @debug
    def sub(a, b):
        return a - b

def debug1(func=None, *, prefix=''):
    if func is None:
        return partial(debug1, prefix=prefix)
    log = logging.getLogger()
    msg = prefix + func.__qualname__
    @wraps(func)
    def wrapper(*args, **kwargs):
        log.critical(msg)
        return func(*args, **kwargs)
    return wrapper

def do_reformation():
    @debug1
    def add0(a, b):
        return a + b

    @debug1(prefix='+++++')
    def add1(a, b):
        return a + b

    print(add0(1, 2))
    print(add1(1, 2))

def do_decorate_cls():    
    def debug_cls(cls):
        for name, value in vars(cls).items():
            if callable(value):
                print(name, value, "is callable")
                setattr(cls, name, debug1(value))
            else:
                print(name, value, "is not callable")
        return cls

    class Foo:
        def foo_a(self):
            pass
        def foo_b(self):
            pass
        def foo_c(self):
            pass
        @classmethod
        def foo_d(cls):
            pass
        @staticmethod
        def foo_e():
            pass


    def debug_getattr(cls):
        ga = cls.__getattribute__
        def wrapper(self, name):
            print("I want ", name)
            return ga(self, name)
        cls.__getattribute__ = wrapper
        return cls

    @debug_getattr
    class Bar:
        def __init__(self):
            self.x = 10
            self.y = 20
        def spam(self):
            pass

    b0 = Bar()
    print(b0.x)
    b0.spam()

def do_meta():
    def debug_attr(cls):
        __getattr__ = cls.__getattribute__
        def wrapper(self, name):
            print("I want", name)
            return __getattr__(self, name)
        cls.__getattribute__ = wrapper
        return cls

    class DebugMeta(type):
        def __new__(cls, clsname, bases, clsdict):
            clsobj = super().__new__(cls, clsname, bases, clsdict)
            clsobj = debug_attr(clsobj)
            return clsobj

    class Foo(metaclass=DebugMeta):
        def __init__(self):
            self.x = 100
            self.y = 200

        def spam(self):
            pass

    class Bar(metaclass=DebugMeta):
        def __init__(self):
            self.x = 300
            self.y = 400

        def spam(self):
            pass

    f = Foo()
    print(f.x)
    f.spam()

    b = Bar()
    print(b.x)
    b.spam()

def do_cls_create():
    class Foo():
        def __init__(self):
            self.name = 'Foo'
        def spam():
            pass

    s = '''
def __init__(self):
    self.name = 'Foo'
def spam():
    pass
        '''
    clsdict = {}
    exec(s, globals(), clsdict)
    print(clsdict)


def do_test5():
    from functools import wraps
    import time

    class Test5:
        def __init__(s, val0, val1):
            s.val0 = val0
            s.val1 = val1
        def show(kk):
            print(kk.val0, kk.val1)

    t0 = Test5(1, 2)
    t0.show()
    print(t0.__dict__)

    def mydec(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            start = time.time()
            ret = func(*args, **kwargs)
            end = time.time() - start
            print(end)
            return ret
        return wrapper

    @mydec
    def loop(n):
        for i in range(n):
            for j in range(n):
                d = 10 + 20 + 30

    loop(10000)


class P3Meta(TestCase):
    def test_test(self):
        do_test5()
        print('Test Done'.center(50, '-'))

if __name__ == '__main__':
    main()
