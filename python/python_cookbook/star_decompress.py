#!/usr/bin/python3

from unittest import TestCase, main

def mysum(l):
    head, *others = l
    return head + sum(others) if others else head

def test_mysum():
    print(mysum([x for x in range(100)]))

def test_dict():
    mydict = {'x': 1, 'y': 2, 'z': 3}
    print(mydict.keys())
    print(mydict.items())
    print(mydict.values())

def dedupe(items, key=None):
    seen = set()
    for item in items:
        val = item if key is None else key(item)
        if val not in seen:
            yield val
            seen.add(val)

def named_slice():
    record = '....................100 .......513.25 ..........'
    SHARE = slice(20, 23)
    PRICE = slice(31, 37)
    print(int(record[SHARE]) * float(record[PRICE]))

    s = "HelloWorld"
    a = slice(5, 200, 2)
    # a.indices(len(s)))   # clipped to (5, 10, 2)
    for i in range(*a.indices(len(s))): # '*' unpacks the sequence/collection into positional arguments
        print(s[i])

def most_common():
    words = [
            'look', 'into', 'my', 'eyes', 'look', 'into', 'my', 'eyes',
            'the', 'eyes', 'the', 'eyes', 'the', 'eyes', 'not', 'around', 'the',
            'eyes', "don't", 'look', 'around', 'the', 'eyes', 'look', 'into',
            'my', 'eyes', "you're", 'under'
            ]
    from collections import Counter
    word_counts = Counter(words)
    top_three = word_counts.most_common(3)
    print(top_three)
    print(word_counts)

class MyTestCase(TestCase):
    def test_dedupe(self):
        tmp = [x for x in range(10)]
        tmp_dict = tmp[:]
        tmp_dict.extend(tmp)
        print(tmp_dict)
        tmp_dict = list(dedupe(tmp_dict))
        self.assertEqual(tmp_dict, tmp)

        tmp = {'a': 1, 'b': 2, 'c': 3, 'd': 1}
        for key, val in tmp.items():
            print((key, val))
        val = list(dedupe(tmp, key=lambda d: tmp[d]))
        print(val)

    def test_name_slice(self):
        named_slice()

    def test_most_common(self):
        most_common()

if __name__ == "__main__":
    main()
