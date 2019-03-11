#!/usr/bin/python

import sys

class Dog(object):  #python2.7 Style
#class Dog():        #python3 Style
    def __init__(self, name, age):
        self.name = name
        self.age = age
    def sit(self):
        print(self.name.title() + " is now sitting")
    def roll_over(self):
        print(self.name.title() + " is now rolling over")

class SheepDog(Dog):
    def __init__(self, name, age, weight=0, height=0):
        super(SheepDog, self).__init__(name, age)  #python2.7 style
        #super().__init__(name, age)                 #python3 style
        self.weight = weight
        self.height = height

    def get_weight(self):
        return self.weight

    def get_height(self):
        return self.height

try:
    with open("0305.txt") as f:
        pass
except Exception as e:
    print(str(e))
else:
    print("No excpetion happened")

