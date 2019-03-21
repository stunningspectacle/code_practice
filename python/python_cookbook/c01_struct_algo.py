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

def use_dedupe():
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

def item_getter():
    from operator import itemgetter

    rows = [{'fname': 'Brian', 'lname': 'Jones', 'uid': 1003},
            {'fname': 'David', 'lname': 'Beazley', 'uid': 1002},
            {'fname': 'John', 'lname': 'Cleese', 'uid': 1001},
            {'fname': 'Big', 'lname': 'Jones', 'uid': 1004}]

    row_by_name = sorted(rows, key=itemgetter("fname"))
    row_by_lname = sorted(rows, key=itemgetter("lname"))
    row_by_uid = sorted(rows, key=lambda t: (t["uid"], t["fname"])) # itemgetter is faster than lambda

    print("row_by_name:", row_by_name)
    print("row_by_lname:", row_by_lname)
    print("row_by_uid:", row_by_uid)

def group_by():
    from operator import itemgetter
    from itertools import groupby

    rows = [{'address': '5412 N CLARK', 'date': '07/01/2012'},
            {'address': '5148 N CLARK', 'date': '07/04/2012'},
            {'address': '5800 E 58TH', 'date': '07/02/2012'},
            {'address': '2122 N CLARK', 'date': '07/03/2012'},
            {'address': '5645 N RAVENSWOOD', 'date': '07/02/2012'},
            {'address': '1060 W ADDISON', 'date': '07/02/2012'},
            {'address': '4801 N BROADWAY', 'date': '07/01/2012'},
            {'address': '1039 W GRANVILLE', 'date': '07/04/2012'}]

    rows.sort(key=itemgetter("date")) #the same items have to be together!

    for date, item in groupby(rows, key=itemgetter("date")):
        print(date)
        for i in item:
            print("     ", i)

def default_dict():
    from collections import defaultdict

    rows = [{'address': '5412 N CLARK', 'date': '07/01/2012'},
            {'address': '5148 N CLARK', 'date': '07/04/2012'},
            {'address': '5800 E 58TH', 'date': '07/02/2012'},
            {'address': '2122 N CLARK', 'date': '07/03/2012'},
            {'address': '5645 N RAVENSWOOD', 'date': '07/02/2012'},
            {'address': '1060 W ADDISON', 'date': '07/02/2012'},
            {'address': '4801 N BROADWAY', 'date': '07/01/2012'},
            {'address': '1039 W GRANVILLE', 'date': '07/04/2012'}]
    rows_by_date = defaultdict(list)
    for row in rows:
        rows_by_date[row["date"]].append(row)
    print("rows_by_date", rows_by_date)

def do_filter():
     mylist = [1, 4, -5, 10, -7, 2, 3, -1]
     clip_neg = [x if x > 0 else 0 for x in mylist]
     print(clip_neg)

     def helper(x):
         if x > 0:
             return True
         return False

     clip_neg0 = list(filter(helper, mylist))
     print(clip_neg0)

def do_compress():
    from itertools import compress

    addresses = [
            '5412 N CLARK',
            '5148 N CLARK',
            '5800 E 58TH',
            '2122 N CLARK',
            '5645 N RAVENSWOOD',
            '1060 W ADDISON',
            '4801 N BROADWAY',
            '1039 W GRANVILLE',
            ]
    counts = [0, 3, 10, 4, 1, 7, 6, 1]

    yes_no = [n > 5 for n in counts]
    good_addr = list(compress(addresses, yes_no))
    print(good_addr)

def do_dict_comprehension():
    prices = {
            'ACME': 45.23,
            'AAPL': 612.78,
            'IBM': 205.55,
            'HPQ': 37.20,
            'FB': 10.75
            }
    tech_names = { 'AAPL', 'IBM', 'HPQ', 'MSFT' }


    p1 = { key: value for key, value in prices.items() if value > 80 }
    print(p1)

    p2 = { key:prices[key] for key in prices.keys() & tech_names } # Use '&' !!
    print(p2)

def do_namedtuple():
    from collections import namedtuple

    Subscriber = namedtuple('Subscriber', ['addr', 'joined'])
    sub = Subscriber('jonesy@example.com', '2012-10-19')
    print(sub.addr, sub.joined)

def do_chainmap():
    from collections import ChainMap

    a = {'x': 1, 'z': 3 }
    b = {'y': 2, 'z': 4 }

    c = ChainMap(a, b)

    print(c)
    print(c['x'])
    print(c['y'])
    print(c['z'])
    print(list(c.keys()))
    print(list(c.values()))

class MyTestCase(TestCase):
    def test(self):
        do_chainmap()

if __name__ == "__main__":
    main()
