#!/usr/bin/python3

from unittest import TestCase, main

def do_str_repr():
    class Pair():
        def __init__(self, x, y):
            self.x = x
            self.y = y

        def __repr__(self):
            return 'Pair({0.x!r}, {0.y!r})'.format(self)
        def __str__(self):
            #return '(%d, %d)' % (self.x, self.y)
            return '({0.x!s}, {0.y!s})'.format(self)

    p = Pair(5, 6)

    print(p)

def do_enter_exit():
    from socket import socket, AF_INET, SOCK_STREAM

    class LazyConnection:
        def __init__(self, address, family=AF_INET, type=SOCK_STREAM):
            self.address = address
            self.family = family
            self.type = type
            self.sock = None
        def __enter__(self):
            if self.sock is not None:
                raise RuntimeError('Already connected')
            self.sock = socket(self.family, self.type)
            self.sock.connect(self.address)
            return self.sock
        def __exit__(self, exc_ty, exc_val, tb):
            self.sock.close()
            self.sock = None

    from functools import partial
    conn = LazyConnection(('www.intel.com', 80))
    # Connection closed
    with conn as s:
        # conn.__enter__() executes: connection open
        s.send(b'GET /index.html HTTP/1.0\r\n')
        s.send(b'Host: www.intel.com\r\n')
        s.send(b'\r\n')
        resp = b''.join(iter(partial(s.recv, 8192), b''))
        # conn.__exit__() executes: connection closed

def do_reduce_mem():
    class Date():
        __slots__ = ['year', 'month', 'day', 'time']
        def __init__(self, year, month, day):
            self.year = year
            self.month = month
            self.day = day
    
    d0 = Data(1985, 2, 11)

def do_property():
    import math
    class Circle:
        def __init__(self, radius):
            self.radius = radius
        @property
        def area(self):
            return math.pi * self.radius ** 2
        @property
        def diameter(self):
            return self.radius ** 2
        @property
        def perimeter(self):
            return 2 * math.pi * self.radius

    class Person:
        def __init__(self, first_name):
            self.set_first_name(first_name)
        # Getter function
        def get_first_name(self):
            return self._first_name
        # Setter function
        def set_first_name(self, value):
            if not isinstance(value, str):
                raise TypeError('Expected a string')
            self._first_name = value
        # Deleter function (optional)
        def del_first_name(self):
            raise AttributeError("Can't delete attribute")
        # Make a property from existing get/set methods
        name = property(get_first_name, set_first_name, del_first_name)

    c0 = Circle(10)
    print(c0.area)

    p0 = Person("Leo")
    print(p0.name)
    p0.name = "John"
    print(p0.name)

def do_super():
    class A:
        def spam(self):
            print('A.spam')
            super().spam()
    class B:
        def spam(self):
            print('B.spam')
    
    class D:
        def myspam(self):
            print('D.myspam')

    class C(A, B, D):
        def spam(self):
            print('C.spam')
        def myspam(self):
            super().myspam()
    
    c0 = C()
    print(C.__mro__)
    c0.spam()
    c0.myspam()

def do_simple_struct():
    import math
    class Structure1:
        # Class variable that specifies expected fields
        _fields = []
        def __init__(self, *args):
            if len(args) != len(self._fields):
                raise TypeError('Expected {} arguments'.format(len(self._fields)))
            # Set the arguments
            for name, value in zip(self._fields, args):
                setattr(self, name, value)
    
    # Example class definitions
    class Stock(Structure1):
        _fields = ['name', 'shares', 'price']
    class Point(Structure1):
        _fields = ['x', 'y']
    class Circle(Structure1):
        _fields = ['radius']
        def area(self):
            return math.pi * self.radius ** 2

    s0 = Stock('ACME', 50, 91.1)
    print(s0)
    print(s0.__dict__)

def do_abc():
    from abc import ABCMeta, abstractmethod

    class IStream(metaclass=ABCMeta):
        @abstractmethod
        def read(self, maxbytes=-1):
            pass
        @abstractmethod
        def write(self, data):
            pass

    class SocketIStream(IStream):
        def read(self, maxbytes=-1):
            pass
        def write(self, data):
            pass

    s0 = SocketIStream()
    if not isinstance(s0, IStream):
        print("no")
    else:
        print("yes")

    import io
    with open("a.txt", "w") as f:
        if isinstance(f, IStream):
            print("before register yes")
        else:
            print("before register no")
    
    IStream.register(io.IOBase)
    with open("a.txt", "w") as f:
        if isinstance(f, IStream):
            print("after register yes")
        else:
            print("after register no")

def do_attr_proxy():
    class A:
        def spam(self, x):
            print("spam")
        def foo(self):
            print("foo")

    class B:
        def __init__(self):
            self._a = A()
        def __getattr__(self, name):
            return getattr(self._a, name)

    b0 = B()
    print(b0.__dict__)
    b0.spam(10)
    b0.foo()

    class C(A):
        def __init__(self):
            #self.a = 0
            pass
        def foo(self):
            print("foo")
        def bar(self):
            print("bar")
        '''
        def __getattr__(self, name):
            print("no attr " + str(name))
        def __setattr__(self, name, value):
            print("no attr " + str(name))
            '''

    c0 = C()
    print("c0.__dict__:", c0.__dict__)
    print(C.__mro__)
    c0.foo()
    #print(c0.ok)
    if callable(c0.foo):
        print("yes")

    c0.a = 10
    print("c0.__dict__:", c0.__dict__)

def do_classmethod():
    import time

    class Date:
        def __init__(self, year, month, day):
            self.year = year
            self.month = month
            self.day = day

        @classmethod
        def today(cls):
            t = time.localtime()
            return cls(t.tm_year, t.tm_mon, t.tm_mday)

    d0 = Date(1982, 2, 11)
    print(d0)
    d1 = Date.today()
    print(d1)

def do_state_machine():
    class State:
        @staticmethod
        def open(self, conn):
            raise NotImplementedError;
        @staticmethod
        def close(self, conn):
            raise NotImplementedError;
        @staticmethod
        def read(self, conn):
            raise NotImplementedError;

    class ClosedState(State):
        @staticmethod
        def open(conn):
            conn.new_state(OpenState)
        @staticmethod
        def close(conn):
            raise RuntimeError("Already closed")
        @staticmethod
        def read(conn):
            raise RuntimeError("Not opened")

    class OpenState(State):
        @staticmethod
        def open(conn):
            raise RuntimeError("Aleady opened");
        @staticmethod
        def close(conn):
            conn.new_state(ClosedState)
        @staticmethod
        def read(conn):
            print("reading")

    class Conn:
        def __init__(self):
            self._state = ClosedState
        def new_state(self, state):
            self._state = state
        def open(self):
            self._state.open(self)
        def close(self):
            self._state.close(self)
        def read(self):
            self._state.read(self)

    c0 = Conn()
    try:
        c0.read()
    except Exception as e:
        print(e)
    try:
        c0.close()
    except Exception as e:
        print(e)
    try:
        raise SystemExit("exit")
    except NotImplementedError:
        print("no")
    else:
        print("ok")
    c0.open()
    c0.read()
    c0.close()

def do_call_by_str():
    class Rect:
        def __init__(self, x=0, y=0):
            self.x = x
            self.y = y
        def area(self, add=0):
            return self.x * self.y + add

    r0 = Rect(10, 20)

    a0 = getattr(r0, 'area')()
    print(a0)

    import operator
    r1 = Rect(20, 30)
    r2 = Rect(30, 40)

    a0 = operator.methodcaller('area', 10)(r0)
    a1 = operator.methodcaller('area', 10)(r1)
    a2 = operator.methodcaller('area', 10)(r2)
    print(a0, a1, a2)

def do_visit():
    class Node:
        pass
    class UnaryOperator(Node):
        def __init__(self, operand):
            self.operand = operand
    class BinaryOperator(Node):
        def __init__(self, left, right):
            self.left = left
            self.right = right
    class Add(BinaryOperator):
        pass
    class Sub(BinaryOperator):
        pass
    class Mul(BinaryOperator):
        pass
    class Div(BinaryOperator):
        pass
    class Negate(UnaryOperator):
        pass
    class Number(Node):
        def __init__(self, value):
            self.value = value

    class NodeVisitor:
        def visit(self, node):
            methname = 'visit_' + type(node).__name__
            print("methname:", methname)
            meth = getattr(self, methname, None)
            if meth is None:
                meth = self.generic_visit
            return meth(node)
        def generic_visit(self, node):
            raise RuntimeError('No {} method'.format('visit_' + type(node).__name__))

    class Evaluator(NodeVisitor):
        def visit_Number(self, node):
            return node.value
        def visit_Add(self, node):
            return self.visit(node.left) + self.visit(node.right)
        def visit_Sub(self, node):
            return self.visit(node.left) - self.visit(node.right)
        def visit_Mul(self, node):
            return self.visit(node.left) * self.visit(node.right)
        def visit_Div(self, node):
            return self.visit(node.left) / self.visit(node.right)
        def visit_Negate(self, node):
            return -node.operand

    #t1 = Sub(Number(3), Number(4))
    #t2 = Mul(Number(2), t1)
    #t3 = Div(t2, Number(5))
    t4 = Add(Number(1), Number(2))

    e0 = Evaluator()
    print(e0.visit(t4))

def do_coroutine():
    def helper():
        print("start...")
        for i in range(10):
            value = yield i
            print("in helper:", value, i)
    
    h = helper()
    next(h)
    v = h.send(888)
    print(v)

def do_weakref():
    import weakref

    class Data:
        def __del__(self):
            print("Data: __del__")
    class Node:
        def __init__(self):
            self.parent = None
            self.children = []
            self.data = Data()

        def add_child(self, child):
            self.children.append(child)
            child.parent = weakref.ref(self)

    n0 = Node()
    n0.add_child(Node())
    print("done")

def do_functools():
    from functools import total_ordering
    class Room:
        def __init__(self, name, length, width):
            self.name = name
            self.length = length
            self.width = width
            self.square_feet = self.length * self.width

    @total_ordering
    class House:
        # What total_ordering does:
        #__le__ = lambda self, other: self < other or self == other
        #__ge__ = lambda self, other: not (self < other)
        #__gt__ = lambda self, other: not (self < other or self == other)
        #__ne__ = lambda self, other: not (self == other)

        def __init__(self, name, style):
            self.name = name
            self.style = style
            self.rooms = list()

        @property
        def living_space_footage(self):
            return sum(r.square_feet for r in self.rooms)

        def add_room(self, room):
            self.rooms.append(room)

        def __str__(self):
            return '{}: {} square foot {}'.format(self.name,
                    self.living_space_footage, self.style)

        def __eq__(self, other):
            return self.living_space_footage == other.living_space_footage

        def __lt__(self, other):
            return self.living_space_footage < other.living_space_footage

    h1 = House('h1', 'Cape')
    h1.add_room(Room('Master Bedroom', 14, 21))
    h1.add_room(Room('Living Room', 18, 20))
    h1.add_room(Room('Kitchen', 12, 16))
    h1.add_room(Room('Office', 12, 12))
    h2 = House('h2', 'Ranch')
    h2.add_room(Room('Master Bedroom', 14, 21))
    h2.add_room(Room('Living Room', 18, 20))
    h2.add_room(Room('Kitchen', 12, 16))
    h3 = House('h3', 'Split')
    h3.add_room(Room('Master Bedroom', 14, 21))
    h3.add_room(Room('Living Room', 18, 20))
    h3.add_room(Room('Office', 12, 16))
    h3.add_room(Room('Kitchen', 15, 17))
    houses = [h1, h2, h3]
    print('Is h1 bigger than h2?', h1 > h2) # prints True
    print('Is h2 smaller than h3?', h2 < h3) # prints True
    print('Is h2 greater than or equal to h1?', h2 >= h1) # Prints False
    print('Which one is biggest?', max(houses)) # Prints 'h3: 1101-square-foot Split'
    print('Which is smallest?', min(houses)) # Prints 'h2: 846-square-foot Ranch'

def do_is_factory():
    import logging

    a = logging.getLogger("foo")
    b = logging.getLogger("bar")
    c = logging.getLogger("foo")

    if a is c:
        print("yes")
    else:
        print("no")
    print("-------------------------")

    import weakref
    class Spam:
        def __init__(self, name):
            self.name = name

    spam_dict = weakref.WeakValueDictionary()
    def get_spam(name):
        if name not in spam_dict:
            s = Spam(name)
            spam_dict[name] = s

        return spam_dict[name]
    
    a = get_spam("foo")
    b = get_spam("bar")
    c = get_spam("foo")

    print(a is b)
    print(a is c)
    print(b is c)
    print("-------------------------")

    class MySpam:
        _spam_cache = weakref.WeakValueDictionary()
        def __new__(cls, name):
            print("__new__ is called")
            if name not in cls._spam_cache:
                print(name, "is not in cache")
                self = super().__new__(cls)
                cls._spam_cache[name] = self
            return cls._spam_cache[name]

        def __init__(self, name):
            self.name = name
            print("__init__ is called")

    #print(MySpam.__mro__)
    
    a = MySpam("Dave")
    print("after a")
    b = MySpam("Daveb")
    print("after b")
    c = MySpam("Dave")
    print(a is b)
    print(a is c)
    print(b is c)

    print("-------------------------")
    print("-------------Best cache manager------------------------")
    class CachedSpamManager2:
        _cache = weakref.WeakValueDictionary()

        @classmethod
        def get_spam(cls, name):
            if name not in cls._cache:
                temp = Spam3._new(name)  # Modified creation
                cls._cache[name] = temp
            else:
                temp = cls._cache[name]
            return temp
        @classmethod
        def clear(cls):
            cls._cache.clear()

    class Spam3:
        def __init__(self, *args, **kwargs):
            raise RuntimeError("Can't instantiate directly")

        # Alternate constructor
        @classmethod
        def _new(cls, name):
            self = cls.__new__(cls)
            self.name = name
            return self

    a = CachedSpamManager2.get_spam("foo")
    b = CachedSpamManager2.get_spam("foo")

    print(a is b)
    print(a.name)

class C08Test(TestCase):
    def test_test(self):
        do_is_factory()
        print("finish test")

if __name__ == '__main__':
    main()

