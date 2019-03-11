#!/usr/bin/python

import unittest
from my_module import func0, func1

class MyTestCase(unittest.TestCase):
    def test_func0(self):
        val0 = func0()
        self.assertEqual(val0, VAL0)
    def test_func1(self):
        val1 = func1()
        self.assertEqual(val0, VAL1)

unittest.main()
