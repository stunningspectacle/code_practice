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

def do_type_assert():
    from inspect import signature
    from functools import wraps

    def typeassert(*ty_args, **ty_kwargs):
        def decorator(func):
            sig = signature(func)
            bind_types = sig.bind_partial(*ty_args, **ty_kwargs).arguments

            @wraps(func)
            def wrapper(*args, **kwargs):
                bind_values = sig.bind(*args, **kwargs).arguments
                for name, value in bind_values.items():
                    if name in bind_types:
                        if not isinstance(value, bind_types[name]):
                            raise TypeError('Argument \''+str(value)+
                                    '\' must be '+str(bind_types[name]))
                return func(*args, **kwargs)
            return wrapper
        return decorator

    @typeassert(a=int, c=list)
    def hello(a, b, c=[2, 3]):
        print(a, b, c)

    try:
        hello(8, 8, 'ok')
    except Exception as e:
        print(e)

    sig = signature(hello)
    print(sig.parameters)
    print(sig.parameters['a'])
    print(sig.parameters['a'].name)
    print(sig.parameters['a'].default)
    print(sig.parameters['a'].kind)

    print(sig.parameters['b'])
    print(sig.parameters['c'])
    print(sig.parameters['c'].name)
    print(sig.parameters['c'].default)
    print(sig.parameters['c'].kind)

def do_decorator_class():
    import types
    from functools import wraps

    class Profiled:
        def __init__(self, func):
            wraps(func)(self)
            self.ncalls = 0

        def __call__(self, *args, **kwargs):
            print('__call__ is called')
            self.ncalls += 1
            return self.__wrapped__(*args, **kwargs)

        def __get__(self, instance, cls):
            print('__get__ is called, instance:', instance, cls)
            if instance is None:
                print('instance is None')
                return self
            t = types.MethodType(self, instance)
            print('t:', t)
            return t

    @Profiled
    def add(x, y):
        return x + y

    class Spam:
        @Profiled
        def bar(self, x):
            print(self, x)

    add(2, 3)
    add(2, 3)
    print(add.ncalls)

    s = Spam()
    print('get s')
    s.bar(1)
    #print(add.ncalls)

    try:
        s.grok(s, 100)
    except Exception as e:
        print(e)

    def grok(self, x):
        print(x)
    print(grok.__get__(s, Spam))

    try:
        grok(s, 200)
    except Exception as e:
        print(e)

def do_decorator_add_arg():
    from functools import wraps
    import inspect

    def optional_debug(func):
        sig0 = inspect.getfullargspec(func).args
        print(sig0)

        sig1 = inspect.signature(func)
        #help(sig1)
        print(sig1)
        print('sig1:', sig1.parameters.keys())
        print('sig1:', sig1.parameters.values())
        if 'a' in sig0:
            print('yes')
        @wraps(func)
        def wrapper(*args, debug=False, **kwargs):
            if debug:
                print('debug is enabled')
            return func(*args, **kwargs)
        return wrapper

    @optional_debug
    def hello(a, b, c):
        print(a, b, c)

    hello(1, 2, 3, debug=True)

def do_class_decorator():
    def logged_getattribute(cls):
        old_attr = cls.__getattribute__
        def wrapper(self, name):
            print('getting:', name)
            return old_attr(self, name)
        cls.__getattribute__ = wrapper
        return cls

    @logged_getattribute
    class A:
        def __init__(self, val):
            self.x = val
        def spam(self):
            pass

    a0 = A(100)
    print(a0.x)
    a0.spam()

    #Use inheritance
    class B:
        def __init__(self, val):
            self.x = val
        def spam(self):
            pass

    class LoggedB(B):
        def __getattribute__(self, name):
            print('b getting:', name)
            return super().__getattribute__(name)

    b0 = LoggedB(200)
    print(b0.x)
    b0.spam()

def do_metaclass():
    class NoInstance(type):
        def __call__(self):
            raise TypeError('No instance should be created')

    class Spam(metaclass=NoInstance):
        @staticmethod
        def grok():
            print('Spam.grok()')

        @classmethod
        def good(cls):
            print(cls)

    Spam.grok()
    Spam.good()
    try:
        s = Spam()
    except Exception as e:
        print(e)

    #----------------------------------

    class Singleton(type):
        def __init__(self, *args, **kwargs):
            print('s __init__ is called')
            self.__inst = None
            super().__init__(*args, **kwargs)

        def __call__(self, *args, **kwargs):
            print('s __call__ is called')
            if self.__inst is None:
                self.__inst = super().__call__(*args, **kwargs)
            return self.__inst

    class Spam(metaclass=Singleton):
        def __init__(self):
            print('Spam __init__')
        def grok(self):
            print('grok()')

    class A:
        def __init__(self, *args, **kwargs):
            print('A __init__ is called')

    class B(metaclass=A): # A's __init__ will be called when interpreter gets here
        pass

    class C(metaclass=A): # A's __init__ will be called when interpreter gets here
        pass

    class D(A): # A's __init__ won't be called until one D's instance is created
        pass

    s0 = Spam()
    s1 = Spam()

    if s0 is s1:
        print('yes')
    else:
        print('no')

    #---------------------------------------

    import weakref
    class Cached(type):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self.__cached = weakref.WeakValueDictionary()
        def __call__(self, *args):
            if args not in self.__cached:
                obj = super().__call__(*args) 
                self.__cached[args] = obj
            return self.__cached[args]

    class Spam(metaclass=Cached):
        def __init__(self, *args, **kwargs):
            print('Create a Spam')

    s0 = Spam('kk')
    s1 = Spam('jj')
    s2 = Spam('kk')
    
    if s0 is s2:
        print('yes')
    else:
        print('no')

def do_class_ordered_attr():
    from collections import OrderedDict

    class Typed:
        _expected_type = type(None)
        def __init__(self, name=None):
            self._name = name

        def __set__(self, instance, value):
            if not isinstance(value, self._expected_type):
                raise TypeError('Expected ' + str(self._expected_type))
            instance.__dict__[self._name]  = value

    class Integer(Typed):
        _expected_type = int
    class Float(Typed):
        _expected_type = float
    class String(Typed):
        _expected_type = str

    class OrderedMeta(type):
        def __new__(cls, clsname, bases, clsdict):
            print('OrderedMeta.__new__:')
            print('cls:', cls)
            print('clsname:', clsname)
            print('bases:', bases)
            print('clsdict:', clsdict)
            d = dict(clsdict)
            print('d:', d)
            order = []
            for name, value in clsdict.items():
                if isinstance(value, Typed):
                    value._name = name
                    order.append(name)
            d['_order'] = order
            return type.__new__(cls, clsname, bases, d)

        @classmethod
        def __prepare__(cls, clsname, bases):
            print('__prepare__() is called')
            return OrderedDict()

    class Structure(metaclass=OrderedMeta):
        def as_csv(self):
            return ','.join(str(getattr(self, name)) for name in self._order)

    class Stock(Structure):
        name = String()
        shares = Integer()
        price = Float()

        def __init__(self, name, shares, price):
            self.name = name
            self.shares = shares
            self.price = price

    s = Stock('GOOG', 100, 490.1)
    print(s.name)
    print(s.as_csv())

    try:
        s = Stock('AAPL', 'a lot', 610.23)
        print(s.name)
        print(s.as_csv())
    except Exception as e:
        print(e)

def do_variable_args_metaclass():
    class A(type):
        def __new__(cls, clsname, bases, ns):
            print('A __new__')
            return super().__new__(cls, clsname, bases, ns)

        @classmethod
        def __prepare__(cls, clsname, bases):
            print('A __parepare__')
            return super().__prepare__()

    class B(metaclass=A):
        pass

def do_limit_class():
    class MyMeta(type):
        def __init__(self, clsname, base, clsdict):
            print('__init__:', self)
            for name in clsdict:
                if name.lower() != name:
                    raise TypeError('Bad attribute name: ' + name)
            super().__init__(clsname, base, clsdict)
        def __new__(cls, clsname, base, clsdict):
            print('__new__:', cls)
            return super().__new__(cls, clsname, base, clsdict)

        @classmethod # Must be classmethod
        def __prepare__(cls, clsname, base):
            print('__prepare__:', cls, clsname, base)
            return super().__prepare__(cls, clsname, base)

    class Root(metaclass=MyMeta):
        pass

    try:
        class A(Root):
            numOne = 0

        class B(Root):
            numtwo = 0

        a = A()
    except Exception as e:
        print(e)

    class D:
        pass
    class DD(D):
        def __init__(self):
            s0 = super()
            print('s0:', s0)

            s1 = super(self.__class__, self)
            print('s1:', s1)
            print('self:', self)
            print('self.__class__:', self.__class__)
            super().__init__()

    dd = DD()
    print('mro:', DD.__mro__)
    #--------------------------------------------
    from inspect import signature
    class SigMatchMeta(type):
        def __init__(self, clsname, bases, clsdict):
            print('clsdict:', clsdict)
            super().__init__(clsname, bases, clsdict)
            sup = super(self, self)
            sup0 = super()
            print('self:', self)
            print('super(self, self):', sup)
            print('sup0()', sup0)
            for name, value in clsdict.items():
                if name.startswith('_') or not callable(value):
                    continue
                attr = getattr(sup, name, None)
                if attr:
                    sig0 = signature(value)
                    sig1 = signature(attr)
                    if sig0 != sig1:
                        raise TypeError(sig0, '!=', sig1)
    class SigRoot(metaclass=SigMatchMeta):
        def spam(x, y, z):
            pass

    class A(SigRoot):
        def spam(x, y):
            pass

def do_customize_class_in_define():
    import operator
    class StructTupleMeta(type):
        def __init__(cls, *args, **kwargs):
            super().__init__(*args, **kwargs)
            for n, name in enumerate(cls._fields):
                setattr(cls, name, property(operator.itemgetter(n)))
    class StructTuple(tuple, metaclass=StructTupleMeta):
        _fields = []
        def __new__(cls, *args):
            if len(args) != len(cls._fields):
                raise ValueError('{} arguments required'.format(len(cls._fields)))
            return super().__new__(cls, args)

    class Stock(StructTuple):
        _fields = ['name', 'shares', 'price']
    
    s0 = Stock('GOOG', 900, 100.0)
    print(s0[0])
    print(s0.name)

def do_param_override():
    class Spam:
        def bar(self, x: int, y: int):
            print(x, y)
        def bar(self, x: str, y: int=10):
            print(x, y)

    s = Spam()
    s.bar(1, 2)
    s.bar('hello')

def do_avoid_duplicate_methods():
    class Person:
        def __init__(self, name, age):
            self._name = name
            self._age = age

        @property
        def name(self):
            return self._name
        @name.setter
        def name(self, name):
            if not isinstance(name, str):
                raise TypeError('name should be a string')
            self._name = name

        @property
        def age(self):
            return self._age
        @age.setter
        def age(self, age):
            if not isinstance(age, int):
                raise TypeError('age should be int')
            self._age = age

    p = Person('John', 20)
    print(p.__dict__)
    p.name = 'Jack'
    print(p.__dict__)
    p.age = 50
    print(p.__dict__)

    #------------------------------------------------

    def typed_property(name, expected_type):
        new_prop = '_' + name
        @property
        def prop(self):
            return getattr(self, new_prop)
        @prop.setter
        def prop(self, value):
            if not isinstance(value, expected_type):
                raise TypeError(name, 'should be', str(expected_type))
            setattr(self, new_prop, value)
        return prop
    
    class Person1:
        name = typed_property('name', str)
        age = typed_property('age', int)
        def __init__(self, name, age):
            self.name = name
            self.age = age

    p = Person1('John1', 21)
    print(p.__dict__)
    p.name = 'Jack1'
    print(p.__dict__)
    p.age = 51
    print(p.__dict__)

    #----------------------------------------------
    from functools import partial
    String = partial(typed_property, expected_type=str)
    Integer = partial(typed_property, expected_type=int)
    class Person2:
        name = String('name')
        age = Integer('age')
        def __init__(self, name, age):
            self.name = name
            self.age = age

    p = Person2('John2', 22)
    print(p.__dict__)
    p.name = 'Jack2'
    print(p.__dict__)
    p.age = 52
    print(p.__dict__)

def do_context_manager():
    import time
    from contextlib import contextmanager

    @contextmanager
    def timethis(label):
        start = time.time()
        try:
            yield
        finally:
            end = time.time()
            print('{}: costs {}'.format(label, end - start))

    with timethis('counting'):
        end = 100000
        time.sleep(0.1)
        while end > 0:
            end -= 1
        
    @contextmanager
    def list_trans(orig_list):
        working_list = list(orig_list)
        yield working_list
        orig_list[:] = working_list

    nums = [ i for i in range(3)]
    print('before:', nums)
    with list_trans(nums) as n:
        n.append(5)
        n.append(6)
    print('after:', nums)

    nums_except = [i for i in range(5)]
    print('before excecption:', nums_except)
    try:
        with list_trans(nums_except) as n:
            n.append(10)
            n.append(11)
            raise Exception('excpetion happens in context')
    except:
        pass
    print('after excecption:', nums_except)

    #------------------------------------------------------
    class Timethis:
        def __init__(self, lable):
            self._lable = lable
        def __enter__(self):
            self._start = time.time()
        def __exit__(self, exc_ty, exc_val, exc_tb):
            print('{}: costs {} seconds'.format(self._lable,
                time.time()-self._start))

    with Timethis('Timethis says'):
        time.sleep(0.5)
        
class C09TestCase(TestCase):
    def test_main(self):
        do_context_manager()
        print('Test Completed'.center(60, '-'))

if __name__ == '__main__':
    main()
