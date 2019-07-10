#!/usr/bin/python3

from unittest import TestCase, main

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
        items = [1, 2, 3, 4, 5]
        def __getitem__(self, index):
            if index > len(items):
                raise IndexError(index, "> ", len(items))
            return items[index]
        def __setitem__(self, index, value):
            if index > len(items):
                raise IndexError(index, "> ", len(items))
            items[index] = value
        def __delitem__(self, index):
            del items[index]
        def __contains__(self, value):
            if value in items:
                return True
            return False

            


class P3Meta(TestCase):
    def test_test(self):
        do_test1()
        print('Test Done'.center(50, '-'))

if __name__ == '__main__':
    main()
