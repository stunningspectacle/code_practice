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

class C08Test(TestCase):
    def test_test(self):
        do_reduce_mem()


if __name__ == '__main__':
    main()

