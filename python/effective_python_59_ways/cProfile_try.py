#!/usr/bin/python
import cProfile
import pstats

def core():
    a = 0
    while True:
        a += 1
        if a >= 10000:
            break

def test0():
    for i in range(100):
        core()

def test1():
    for i in range(10):
        core()

def test3(): 
    for i in range(10):
        test0()


def test4():
    for i in range(20):
        test1()


def test():
    test3()
    test4()

profiler = cProfile.Profile()
profiler.runcall(test)

stats = pstats.Stats(profiler)
stats.strip_dirs()
stats.sort_stats('cumulative')
stats.print_stats()
stats.print_callers()

