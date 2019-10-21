#!/usr/bin/python3 

class A:
    _num = 100

if __name__ == '__main__':
    a = 100
    b = a 
    print(a, b)

    a = 200
    print(a, b)

    c = [100]
    d = c
    print(c, d)

    c.append(200)
    print(c, d)


    f = A()
    print(f)
    g = f
    print(g)
