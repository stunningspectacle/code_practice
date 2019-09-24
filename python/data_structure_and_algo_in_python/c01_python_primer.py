#!/usr/bin/python3

from unittest import TestCase, main
import inspect

default_points = {  'A+': 4.0, 'A': 4.0, 'A-': 3.67,
            'B+': 3.33, 'B': 3.0, 'B-': 2.67,
            'C+': 2.33, 'C': 2.0, 'C-': 1.67,
            'D+': 1.33, 'D': 1.0, 'F': 0.0 }

def del_list(l):
    del l[0 : len(l) // 2]
def def_param(a, *, key0, key1):
    print(a, key0, key1)
def mymax(*args, key=None):
    m = args[0]
    for arg in args[1:]:
        if key is not None:
            if key(m) < key(arg):
                m = arg
        else:
            if m < arg:
                m = arg

    return m
def factor(n):
    for k in range(1, n + 1):
        if n % k == 0:
            yield k
def factor2(n):
    k = 1
    while k * k < n:
        if n % k == 0:
            yield k
            yield n // k
        k += 1
    if k * k == n:
        yield k

def fibonacci(n):
    a = 0
    b = 1
    count = 0
    while True:
        yield a
        c = a + b
        a = b
        b = c
        count += 1
        if count == n:
            return

def fibonacci2(n):
    count = 0
    a, b = 0, 1
    while count < n:
        yield a
        a, b = b, a + b
        count += 1

class MyTest(TestCase):
    def test_1(self):
        temp = 80
        copy = temp
        temp += 30
        print(copy, temp)

    def test_2(self):
        print()

        x = bool(100)
        print(x)
        l0 = []
        print(bool(l0))
        l0.append(10)
        print(bool(l0))

    def test_3(self):
        print()

        l0 = [i for i in range(10)]
        print(l0)
        l1 = list(l0)
        #l1 = l0[:]
        #l1 = l0[0:len(l0)]
        print(l1)
        del l0[0:5]
        print(l0, l1)

    def test_4(self):
        print()

        print(inspect.cleandoc('''
            Welcome to the GPA calculator.
            Please enter all your letter grades, one per line.
            Enter a blank line to designate the end.'''))

    def test_5(self):
        print()

        print(set('hello'))

    def test_6(self):
        print()

        l0 = [i for i in range(20)]
        print(l0)
        del_list(l0)
        print(l0)

    def test_7(self):
        print()

        def_param(10, key0 = 20, key1 = 30)

    def test_gpa(self, points=default_points):
        print()

        done = False
        nr_courses = 0
        total_points = 0.0

        while not done:
            grade = input().upper()
            if grade == '':
                done = True
            elif grade not in points:
                print('Not a valid grade')
            else:
                print('Got one', grade, points[grade])
                nr_courses += 1
                total_points += points[grade]

        if nr_courses > 0:
            print('GPA is', total_points / nr_courses)

    def test_max(self):
        print()

        print('test_max:', max(30, -20, key=lambda x: x if x > 0 else -x))

    def test_print_file(self):
        print()

        with open('a.txt', 'w') as f:
            print([i for i in range(100)], file=f)

    def test_file_read(self):
        print()

        with open(__file__, 'r') as f:
            for idx, val in enumerate(f.readlines()):
                pass
                #print(idx, val, end='')
            
    def test_except(self):
        print()

        try:
            while True:
                val = int(input('Input something:'))
        except ValueError as e:
            print('ValueError:', e)
        except EOFError as e:
            print('EOFError:', e)
            raise
        except Exception as e:
            print('other errors:', e)

    def test_factor(self):
        print()

        l = list(factor2(100))
        print(id(l), l)
        l = sorted(l)
        print(id(l), l)

    def test_fibonacci(self):
        print()

        print('fibonacci:', list(fibonacci(10)))
        print('fibonacci2:', list(fibonacci2(20)))

    def test_generator_comprehension(self):
        print()

        gen = (k * k for k in range(100))
        print('generator_comprehension:', sum([k * k for k in range(10)]))
        print(sum(gen))

if __name__ == '__main__':
    '''
    points = {  'A+': 14.0, 'A': 14.0, 'A-': 13.67,
                'B+': 13.33, 'B': 13.0, 'B-': 12.67,
                'C+': 12.33, 'C': 12.0, 'C-': 11.67,
                'D+': 11.33, 'D': 11.0, 'F': 0.0 }
    MyTest().test_gpa(points)
    '''
    print(mymax(10, 1, 2, 3, 4, -5, key=abs))
    main()

