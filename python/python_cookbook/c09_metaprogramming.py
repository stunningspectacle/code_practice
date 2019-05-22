#!/usr/bin/python3

from unittest import TestCase, main

def do_decorator():

    def timethis(func):
        from functools import wraps
        from time import time
        @wraps(func)
        def wrapper(*args, **kwargs):
            start = time()
            res = func(*args, **kwargs)
            cost = time() - start
            print('{}(): called at {}, cost {} secs, ret {}'.format(func.__name__, start, cost, res))
            return res
        return wrapper 

    @timethis
    def say_hello(n:int) -> int:
        ''' this is just a sample'''
        print('hello there!', n)
        return n

    say_hello(100)
    print(say_hello.__doc__)
    print(say_hello.__name__)
    print(say_hello.__annotations__)

    from inspect import signature
    print(signature(say_hello))

def do_strip_decorator():
    def mydec(func):
        from functools import wraps
        @wraps(func)
        def wrapper(*args, **kwargs):
            print('my decorator')
            res = func(*args, **kwargs)
            return res
        return wrapper

    @mydec
    def hello(n=15):
        print('helloc', n)

    hello()

    h0 = hello.__wrapped__
    h0()

def do_decorator_with_args():
    import logging
    from functools import wraps

    def mylog(level, name=None, message=None):
        def decorator(func):
            logname = name if name else func.__module__
            msg = message if message else func.__name__
            log = logging.getLogger(logname)

            @wraps(func)
            def wrapper(*args, **kwargs):
                log.log(level, msg)
                return func(*args, **kwargs)
            return wrapper
        return decorator

    @mylog(logging.DEBUG, name='yeshello0')
    def hello0(n=100):
        print('hello0', n)

    @mylog(logging.CRITICAL, name='yeshello1', message='bbbbb')
    def hello1(n=101):
        print('hello1', n)

    print('module:', hello0.__module__)
    hello0()

    hello1()

    h = hello1.__wrapped__
    mylog(logging.CRITICAL, message='kkkkk')(h)(999)

def do_customized_decorator():
    import logging
    from functools import partial, wraps

    def attach_wrapper(obj, func=None):
        print('attach_wrapper() called', obj, func)
        if func is None:
            return partial(attach_wrapper, obj)
        setattr(obj, func.__name__, func)
        return func

    def logged(level, name=None, message=None):
        def decorate(func):
            logname = name if name else func.__module__
            logmsg = message if message else func.__name__
            logmsg += ' logmsglogmsg '
            logger = logging.getLogger(logname)

            @wraps(func)
            def wrapper(*args, **kwargs):
                logger.log(level, logmsg)
                return func(*args, **kwargs)
            
            @attach_wrapper(wrapper)
            def set_level(newlevel):
                nonlocal level
                level = newlevel

            @attach_wrapper(wrapper)
            def set_msg(msg):
                nonlocal logmsg
                logmsg = msg

            print('set_level', set_level)
            print('set_msg', set_msg)
            return wrapper

        return decorate

    @logged(logging.CRITICAL)
    def hello():
        print('hello', 111)

    print('hello', hello)
    print('attach_wrapper', attach_wrapper)
    hello()

def do_variable_decorator():
    import logging
    from functools import partial, wraps

    def logged(func=None, *, level=logging.CRITICAL):
        if func is None:
            return partial(logged, level=level)
        logger = logging.getLogger('sample')

        @wraps(func)
        def wrapper(*args, **kwargs):
            logger.log(level, 'aaaaa')
            return func(*args, **kwargs)
        return wrapper

    @logged
    def hello0():
        print('hello0')

    @logged(level=logging.DEBUG)
    def hello1():
        print('hello1')

    hello0()
    hello1()

class C09TestCase(TestCase):
    def test_main(self):
        do_variable_decorator()
        print('Test Completed'.center(60, '-'))

if __name__ == '__main__':
    main()
