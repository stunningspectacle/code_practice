#!/usr/bin/python3

from unittest import TestCase, main


def do_var_args():
    def var_args(x, *tuple_args, y=100, **dict_args):
        print(tuple_args) # tuple
        print(*tuple_args) # members decompressed from tuple

        print(dict_args)
        print(*dict_args)

        keyvals = ['%s=%s' % item for item in dict_args.items()]
        print(keyvals)
        attr_str = ''.join(keyvals)
        print(attr_str)

        a1 = 10
        b1 = 20
        print('{a}:xxx{b}yyy'.format(a=a1, b=b1))

    var_args(100, 200, 300, y=400, a='a', b='b', c='c')

def do_enforced_key_args():
    def enforced_key_args0(x, *, y):
        print(x, y)
    def enforced_key_args1(*x, y):
        print(x, y)

    enforced_key_args0(100, y=200)
    enforced_key_args1(100, 200, 300, 400, y=500)

def do_add_metainformation():
    def add(x: int, y: int) -> int:
        print(x + y)
    print(add.__annotations__)

def do_default_val_args():
    _no_value = object()

    def func0(a, b=_no_value):
        print("a:", a, ' ', end='')
        if b is _no_value: #should not be 'if not b'
            print("no b is supplied")
        else:
            print("b:", b)
    func0(1, 2)
    func0(1, None)
    func0(1)

def do_inline_anony_func():
    f0 = lambda *x: sum(x)
    print(f0(1, 2, 3, 4))

    names = ['David Beazley', 'Brian Jones', 'Raymond Hettinger', 'Ned Batchelder']

    sorted_name = sorted(names, key=lambda name: name.split()[-1].lower())
    print(sorted_name)

def do_anony_runtime():
    y = 10
    func0 = lambda x: x + y
    y = 20
    func1 = lambda x: x + y
    print(func0(20))
    print(func1(20))

    y = 10
    func0 = lambda x, y=y: x + y
    y = 20
    func1 = lambda x, y=y: x + y
    print(func0(20))
    print(func1(20))

    funcs = [lambda x: x+n for n in range(5)]
    for f in funcs:
        print(f(0))

    funcs = [lambda x, n=n: x+n for n in range(5)]
    for f in funcs:
        print(f(0))

def do_functools_partial():
    from functools import partial

    def spam(a, b, c, d, e):
        print('a:', a, 'b:', b, 'c:', c, 'd:', d, 'e:', e)
    spam(1, 2, 3, 4, 5)

    spam0 = partial(spam, 10)
    spam0(1, 2, 3, 4)

    spam1 = partial(spam, a=10, b=88)
    spam1(1, 2, 3)

def do_single_method_class_to_closure():
    class MyTest():
        def __init__(self, key):
            self.key = key
        def add(self, b):
            print(self.key+b)
    t = MyTest(100)
    t.add(100)

    def myTest(key):
        def add(b):
            nonlocal key # use nonlocal to declare the env variables
            print(key+b)
            key *= 2
        return add

    t = myTest(10)
    t(10)
    t(20)

def do_async_callback():
    def apply_async(func, *args, callback):
        result = func(args)
        callback(result)
    def print_result(res):
        print('got:', res)
    def add(*args):
        return sum(*args)
    apply_async(add, 2, 3, 4, 5, callback=print_result)

    def print_result_closure(seq):
        def handler(res):
            nonlocal seq
            seq += 1
            print('seq:', seq, 'got:', res)
        return handler
    apply_async(add, 2, 3, 4, 5, callback=print_result_closure(0))

    def print_result_coroutine():
        seq = 0
        while True:
            res = yield
            seq += 1
            print('seq:', seq, 'got:', res)
    handler = print_result_coroutine()
    next(handler)
    apply_async(add, 2, 3, 4, 5, callback=handler.send)

def do_my_coroutine():
    def make_coroutine():
        key0 = 0
        key1 = 1
        while True:
            args = yield (1, 3)
            if args:
                res = sum(args) + key0 * key1
                key0 += 10
                key1 += 100
                yield res

    handler = make_coroutine()
    a = handler.send(None)
    print(a)
    a = (handler.send((10, 20, 30)))
    print(a)
    a = (handler.send((10, 20, 30)))
    print(a)

def do_decorator_and_wraps():
    def trace0(func):
        def caller(*args, **kwargs):
            print('{} is called'.format(func.__name__))
            res = func(*args, **kwargs)
            print('{} returns {}'.format(func.__name__, res))
            return res
        return caller

    @trace0
    def square0(x):
        return x * x

    print(square0(100), square0.__name__)
    print(''.center(20, '*'))

    def trace1(func):
        from functools import wraps
        @wraps(func)
        def caller(*args, **kwargs):
            print('{} is called'.format(func.__name__))
            res = func(*args, **kwargs)
            print('{} returns {}'.format(func.__name__, res))
            return res
        return caller

    @trace1
    def square1(x):
        return x * x

    print(square1(100), square1.__name__)

def do_inline_callback():
    from queue import Queue
    from functools import wraps

    def apply_async(func, args, *, callback):
        # Compute the result
        result = func(*args)
        # Invoke the callback with the result
        callback(result)

    class Async:
        def __init__(self, func, args):
            self.func = func
            self.args = args

    def inlined_async(func):
        @wraps(func)
        def wrapper(*args):
            f = func(*args)
            result_queue = Queue()
            result_queue.put(None)
            while True:
                result = result_queue.get()
                try:
                    ''' 
                    work flow:
                    1. first send makes test() reach yield with delivering of Async(add, (2, 3)) and stops
                    2. a is Async(add, (2, 3)), apply_async() do the process and queue the result
                    3. get result from queue
                    4. send result makes test() awake and print(r)
                    5. test() reach second yield with delivering of Async(add, ('hello', 'world')) and stops
                    6. A is Async(add, ('hello', 'world'))
                    '''
                    a = f.send(result) 
                    apply_async(a.func, a.args, callback=result_queue.put)
                except StopIteration:
                    break
        return wrapper

    def add(x, y):
            return x + y

    @inlined_async
    def test():
        r = yield Async(add, (2, 3))
        print(r)
        r = yield Async(add, ('hello', 'world'))
        print(r)
        for n in range(10):
            r = yield Async(add, (n, n))
            print(r)
        print('Goodbye')


    test()

def do_myinline_callback():
    from functools import wraps
    from queue import Queue

    def add(x, y):
        return x + y
    def async_callback(func, args, *, callback):
        res = func(*args)
        callback(res)
    class Async:
        def __init__(self, func, args):
            self.func = func
            self.args = args
    def inline_wrap(func):
        @wraps(func)
        def wrapper(*args):
            f = func(*args)
            q = Queue()
            q.put(None)

            while True:
                try:
                    res = q.get()
                    a = f.send(res)
                    async_callback(a.func, a.args, callback=q.put)
                except Exception as e:
                    print(e)
                    break
        return wrapper

    @inline_wrap
    def test():
        res = yield Async(add, (10, 20))
        print('res', res)
        res = yield Async(add, ('good', 'bad'))
        print(res)
        for i in range(10):
            res = yield Async(add, (i, i))
            print(res)

        print('goodbye')
    
    test()

def do_closure_internal_var():
    import sys

    def show(k):
        n = k
        def func():
            pass
        def set_n(v):
            nonlocal n
            n = v
        def get_n():
            return n

        func.set_n = set_n
        func.get_n = get_n
        return func

    f = show(100)

    print(f.get_n())
    f.set_n(20)
    print(f.get_n())

    print(sys._getframe(0).f_locals)


class C07TestCase(TestCase):
    def test_main(self):
        do_closure_internal_var()
        print(' Test Done '.center(80, '-'))

if __name__ == '__main__':
    main()
