#!/usr/bin/python3

class A:
    def __init__(self, n):
        print("A.__init__")
        self.Avalue = n
        print("Avalue:", Avalue)
    def __del__(self):
        print("A.__del__")

class B(A):
    def __init__(self):
        super().__init__()
        print("B.__init__")
        #self.Bvalue = n
    def __del__(self):
        print("B.__del__")
        super().__del__()

class C(A):
    def __init__(self):
        #super().__init__()
        print("C.__init__")
    def __del__(self):
        print("C.__del__")
        super().__del__()

class D(B, C):
    def __init__(self):
        super().__init__()
        print("D.__init__")
    def __del__(self):
        print("D.__del__")
        super().__del__()

if __name__ == '__main__':
    d = D()
