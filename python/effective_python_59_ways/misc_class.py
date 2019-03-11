#!/usr/bin/python

from datetime import datetime
from collections import Sequence

def generator_express():
    """Use generator expression rather than list comprehension for
    large data
    """
    with open("simple.py") as f:
        print([len(x) for x in f])

    with open("simple.py") as f:
        it = (len(x) for x in f)
        for length in it:
            print(length)

    print("xxxxxxxxxxxxxxxxxxxxxxxx")
    with open("simple.py") as f:
        it = (len(x) for x in f)
        while True:
            try:
                length = next(it)
                print(length)
            except StopIteration:
                print("xxxxxxxxxxxxxx")
                break

    print("done")

def enum_range():
    #Use enumerator when needs index in range

    nums = [11, 33, 44, 22, 66]
    for i, value in enumerate(nums):
        print("%d, %d" % (i, value))

def try_except_else_finally():
    try:
        f = open("../simple.py")
    except Exception as e:
        print(str(e))
    else:
        print(len(f.read()))
        f.close()
    finally:
        print("done!")

def raise_exception():
    try:
        a = 1 / 0
    except Exception as e:
        print(str(e))
        #raise RuntimeError("something wrong") from e #python3 can add "from e" as the cause
        raise RuntimeError("something wrong")        #python2.7
    else:
        print(a)
    print("can we print it out?")

#value in group will be prioritized
def sort_prio(values, group):
    def helper(x, group):
        if x in group:
            return (0, x)
        return (1, x)
    values.sort(key=helper)

def iter_only_works_once():
    it = (x for x in range(10))
    #print(list(it))
    print(sum(it))
    for i in it:
        print(i)

def log(msg, when=None):
    when = datetime.now() if when is None else when
    print("%s - %s" % (when, msg))

class Test0(object):
    def __init__(self):
        self.num0 = 0
        self.num1 = 0

class Test1(Test0):
    def __init__(self, num2=0):
        super(Test1, self).__init__()
        self.num2 = num2

version = 1

def d1(func):
    def f2():
        print("this is f2")

    if version > 0:
        return f2
    return func

@d1
def f1():
    print("this is f1")


class TestStatic(object):
    @staticmethod
    def foo():
        print("this is foo of TestStatic")

class TestClass(object):
    @classmethod
    def foo(cls):
        print("this is foo of " + cls.__name__)

####################################
def test_multi_inherit():
    class Z(object):
        def __init__(self):
            self.z = 0
            print("init Z")

    class A(Z):
        def __init__(self):
            super(A, self).__init__()
            self.a = 0
            print("init A")

    class B(Z):
        def __init__(self):
            super(B, self).__init__()
            self.b = 0
            print("init B")

    class AB(A, B):
        def __init__(self):
            super(AB, self).__init__()
            #A.__init__(self)
            #B.__init__(self)
            print("init AB")

    print AB.mro()
    print(dir(AB()))
    # True: print isinstance(AB(), A)
    # True: print isinstance(AB(), B)
    # True: print isinstance(AB(), Z)
###################################

###################################
class ToDictMixin(object):
    def to_dict(self):
        return self._traverse_dict(self.__dict__)

    def _traverse_dict(self, instance_dict):
        output = { }
        for key, value in instance_dict.items():
            output[key] = _traverse(key, value)
        return output

    def _traverse(self, key, value):
        if isinstance(value, ToDictMixin):
            return value.to_dict()
        elif isinstance(value, list):
            return [self._traverse(key, i) for i in value]
        elif hasattr(value, "__dict__"):
            return self._traverse_dict(value.__dict__)
        else:
            return value
###################################

###################################
class FrequencyList(list):
    def __init__(self, members):
        super(FrequencyList, self).__init__(members)

    def freqency(self):
        counts = {}
        for item in self:
            counts.setdefault(item, 0)
            counts[item] += 1
        return counts

    @staticmethod
    def test():
        fl = FrequencyList([1, 1, 2, 3, 2, 3, 2, "aaa", "bbb", "aaa"])
        print fl.freqency()

        class Abs(object):
            def __init__(self):
                if not hasattr(self, "__len__"):
                    raise TypeError("must implement __len__")

        class BadType(Abs):
            def __init__(self):
                super(BadType, self).__init__()

            def a__len__(self):
                print("I have __len__")

###################################

###################################
class Apple(object):
    def __init__(self, weight=0):
        self._weight = weight

    @property
    def weight(self):
        return self._weight

    @weight.setter
    def weight(self, weight):
        self._weight = weight

    @staticmethod
    def test():
        a = Apple()
        print "a.weight =", a.weight
        a.weight = 10
        print "a.weight =", a.weight
###################################

###################################
class TestAttr(object):
    def __init__(self):
        self.a = 0

    def __getattr__(self, name):
        print "first value for %s" % name, 20
        value = 20
        self.name = value
        return value

    def __getattribute__(self, name):
        print("getting %s" % name)
        try:
            return super(TestAttr, self).__getattribute__(name)
        except AttributeError:
            pass
            #print("no attr for %s, set to 33" % name)
            #setattr(self, name, 33)
            #return 33

    @staticmethod
    def test():
        t = TestAttr()
        print t.a
        print t.b
