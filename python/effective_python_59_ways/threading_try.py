#!/usr/bin/python

from time import time 
from threading import Thread, Lock

def factorize(number):
    for i in range(1, number + 1):
        if number % i == 0:
            yield i

numbers = [213079, 121459, 1615647, 1853434, 43234322]

class TestThread(Thread):
    def __init__(self, number):
        super(TestThread, self).__init__()
        self.number = number

    def run(self):
        self.factors = list(factorize(self.number))

    @staticmethod
    def test0():
        start = time()
        for num in numbers:
            list(factorize(num))
        print("cost %f" % (time() - start))

    @staticmethod
    def test1():
        threads = []
        start = time()
        for number in numbers:
            t = TestThread(number)
            #print(t.isAlive())
            t.start()
            #print(t.isAlive())
            threads.append(t)
        for t in threads:
            t.join()
        print("cost %f" % (time() - start))

# Use a Lock
class LockingCounter(object):
    def __init__(self):
        self.lock = Lock()
        self.count = 0

    def increment(self, offset):
        with self.lock:
            self.count += offset

from queue import Queue
# Use a Queue
def test_queu():
    queue = Queue()

    def consumer():
        print("consumer waiting")
        queue.get()
        print("consumer done")

    thread = Thread(target=consumer)
    thread.start()

    print("Producer putting")
    queue.put(object())
    thread.join()
    print("Producer done")

# Coroutine: yield, next(coroutine), send()
def minimize():
    print("a")
    current = yield
    print("b")
    while True:
        value = yield current
        current = min(current, value)

cr = minimize()
print("first", next(cr))

