#!/usr/bin/python3

from unittest import main, TestCase

def do_csv():
    import csv
    import re

    fn = 'stock.csv'
    with open(fn) as f:
        f_csv = csv.reader(f)
        headers = next(f_csv)
        new_headers = [re.sub('[^a-zA-Z_]', '_', h) for h in headers]
        print('headers:', headers)
        print('new_headers:', new_headers)
        for row in f_csv:
            print(row)

    with open(fn) as f:
        f_csv = csv.DictReader(f)
        headers = next(f_csv)
        print('headers:', headers)
        for row in f_csv:
            print(row)

    from collections import namedtuple
    with open(fn) as f:
        f_csv = csv.reader(f)
        headers = next(f_csv)
        Row = namedtuple('Row', headers)
        print(Row)
        for row in f_csv:
            row = Row(*row)
            print('Symbol: ', row.Symbol)

    header = ('Symbol', 'Price', 'Date', 'Time', 'Change', 'Volume')
    stocks = [
        ("AA",39.48,"6/11/2007","9:36am",-0.18,181800),
        ("AIG",71.38,"6/11/2007","9:36am",-0.15,195500),
        ("AXP",62.58,"6/11/2007","9:36am",-0.46,935000),
        ("BA",98.31,"6/11/2007","9:36am",+0.12,104800),
        ("C",53.08,"6/11/2007","9:36am",-0.25,360900),
        ("CAT",78.29,"6/11/2007","9:36am",-0.23,225400)
        ]
    with open('stock_w.csv', 'w') as f:
        f_csv = csv.writer(f)
        f_csv.writerow(header)
        f_csv.writerows(stocks)

class C06TestCase(TestCase):
    def test_test(self):
        do_csv()
        print(' Test done '.center(80, '*'))

if __name__ == '__main__':
    main()

