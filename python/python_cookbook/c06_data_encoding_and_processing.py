#!/usr/bin/python3
from unittest import main, TestCase

def do_large_xml():
    from xml.etree.ElementTree import iterparse

    with open('rss20.xml', 'rb') as f:
        ip = iterparse(f, ('start', 'end'))
        print('ip:', ip)
        next(ip)

        count = 0
        for event, elem in ip:
            if count > 10:
                break
            if event == 'start':
                print('start:', elem.text)
                print('tag:', elem.tag)
            elif event == 'end':
                print('end:', elem.text)
                print()
                count += 1

def do_xml():
    from urllib.request import urlopen
    from xml.etree.ElementTree import parse

    #u = urlopen('https://planet.python.org/rss20.xml')
    #u = urlopen('http://www.163.com')
    #print(u)
    with open('rss20.xml', 'rb') as f:
        xml_doc = parse(f)
        print(xml_doc)
        e = xml_doc.find('channel/title')
        print(e.tag)
        print(e.text)
        return
        for item in xml_doc.iterfind('channel/item'):
            title = item.findtext('title')
            date = item.findtext('pubDate')
            link = item.findtext('link')

            print(title)
            print(date)
            print(link)
            print() #no arg will print newline


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

def do_dict2xml():
    from xml.etree.ElementTree import Element
    from xml.etree.ElementTree import tostring

    s = { 'name': 'GOOG', 'shares': 100, 'price': 490.1 }
    elem = Element('stock')
    for key, value in s.items():
        child = Element(key)
        child.text = str(value)
        elem.append(child)
    print(tostring(elem))

    elem.set('_id', '1234')
    print(tostring(elem))

def do_change_write_xml():
    from xml.etree.ElementTree import parse, Element

    fn = 'rss20.xml'
    doc = parse(fn)
    print(doc)
    root = doc.getroot()
    print(root.items())

    channel = root.find('channel')
    #channel.remove(channel.find('link'))
    e = Element('spam')
    e.text = 'this is a spam'
    index = channel.getchildren().index(channel.find('title'))
    print("index of title:", index)
    channel.insert(index+1, e)
    doc.write(fn, xml_declaration=True)

def do_bstr_hex_conversion():
    import binascii, base64
    s = b'hello'

    h = binascii.b2a_hex(s)
    print(h)
    s0 = binascii.a2b_hex(h)
    print(s0)

    import base64
    h = base64.b16encode(s)
    print(h)
    s0 = base64.b16decode(h)
    print(s0)

def do_base64():
    import base64

    s = b'helloabcddakdjfkasdjf'
    s0 = base64.b64encode(s)
    print(s0)
    s = base64.b64decode(s0)
    print(s)

def do_struct():
    import os, struct
    records = [ (1, 2.3, 4.5), (6, 7.8, 9.0), (12, 13.4, 56.7) ]

    fn = 'struct.bin'
    record_struct = struct.Struct('<idd')
    mode = ''

    if os.path.exists(fn):
        mode = 'wb'
    else:
        mode = 'xb'
    with open(fn, mode) as f:
        for r in records:
            f.write(record_struct.pack(*r))

    with open(fn, 'rb') as f:
        it = iter(lambda: f.read(record_struct.size), b'')
        chunks = (record_struct.unpack(data) for data in it)
        for chunk in chunks:
            print(chunk)

    with open(fn, 'rb') as f:
        data = f.read()
        chunks = (record_struct.unpack_from(data, offset)
            for offset in range(0, len(data), record_struct.size))
        for chunk in chunks:
            print(chunk)

    from collections import namedtuple

    Record = namedtuple('Record', ['kind', 'x', 'y'])
    with open(fn, 'rb') as f:
        it = iter(lambda: f.read(record_struct.size), b'')
        records = (Record(*record_struct.unpack(data)) for data in it)
        for r in records:
            print('r.kind:', r.kind)

class C06TestCase(TestCase):
    def test_test(self):
        do_struct()
        print(' Test done '.center(80, '*'))

if __name__ == '__main__':
    main()

