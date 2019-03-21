#!/usr/bin/python3

from unittest import TestCase, main

def do_re_split():
    import re

    line = 'asdf fjdk; afed, fjek,asdf, foo'
    r1 = re.split(r'[;, ] *', line) #\s means any whitespace, includes " \t\n\r\f\v"
    print(r1)

def do_starts_ends():
    import os

    files = os.listdir('.')
    good_files = [f for f in files if f.endswith(('.c', '.h', '.py'))]
    print(good_files)
    if good_files:
        print(True)
    else:
        print(False)
    print(any(f for f in good_files if f.startswith('c2')))

def do_match():
    import re
    text = 'Today is 11/27/2012. PyCon starts 3/13/2013.'

    r = re.compile(r'\d+/\d+/\d+')
    dates= r.findall(text)
    print(dates)

    r = re.compile(r'(\d+)/(\d+)/(\d+)') #use () to capture the split
    for d in dates:
        try:
            m = r.match(d)
            print(m)
            print("group():", m.groups)
            print("group(0):", m.group(0))
            print("group(1):", m.group(1))
            print("group(2):", m.group(2))
            print("group(3):", m.group(3))
            print("group(4):", m.group(4))
        except Exception:
            continue

def do_search_replace():
    text = 'yeah, but no, but yeah, but no, but yeah'
    t0 = text.replace('yeah', 'yep')
    print(t0)
    print(text)

    import re
    text = 'Today is 11/27/2012. PyCon starts 3/13/2013.'

    t0 = re.sub(r'(\d+)/(\d+)/(\d+)', r'\3-\1-\2', text)
    print(t0)


    from calendar import month_abbr
    def change_date(m):
        print(m.groups())
        mon_name = month_abbr[int(m.group(1))]
        return '{} {} {}'.format(m.group(2), mon_name, m.group(3))

    r = re.compile(r'(\d+)/(\d+)/(\d+)')
    t0 = r.sub(change_date, text) # use helper func to replace
    print(t0)

def do_no_greedy():
    import re
    text =  'Computer says "no." Phone says "yes."'

    r = re.compile(r'\".*\"')
    print(r.findall(text))

    r = re.compile(r'\".*?\"') # add ? to make * no greedy
    print(r.findall(text))

def do_strip():
    with open("c1_struct_algo.py") as f:
        lines = (line.strip() for line in f) # create an iterator
        for line in lines:
            print(line)
def do_format():
    text = 'Hello World'
    print(text.ljust(20))
    print(text.rjust(20))
    print(text.center(20))
    print(text.ljust(20, '='))

    print(format(text, '>20'))
    print(format(text, '<20'))
    print(format(text, '^20'))
    print(format(text, '=^20'))

    print("{:=^20} {:=^20}".format("hello", "world"))

def do_str_has_var():
    s = '{name} has {n} messages.'
    s0 = s.format(name="leo", n="37")
    print(s0)

def do_textwrap():
    import textwrap
    s = "Look into my eyes, look into my eyes, the eyes, the eyes, \
            the eyes, not around the eyes, don't look around the eyes, \
            look into my eyes, you're under."
    print(s)
    import re
    s = re.sub(r'\s+', ' ', s)
    print(s)

    s0 = textwrap.fill(s, 70)
    print(s0)

    s1 = textwrap.fill(s, 40, initial_indent='  ')
    print(s1)

    s2 = textwrap.fill(s, 50, subsequent_indent='  ')
    print(s2)

def do_get_terminal_size():
    import os

    print(os.get_terminal_size())

def do_html_xml():
     import html
     s = 'Elements are written as "<cs find s>text</cs find s>".'
     print(s)

     s0 = html.escape(s)
     print(s0)
     s1 = html.escape(s, quote=False)
     print(s1)

     s = 'Spicy Jalapeño'
     s0 = s.encode('ascii', errors='xmlcharrefreplace')
     print(s0)

     s = 'Spicy &quot;Jalape&#241;o&quot.'
     from html.parser import HTMLParser
     p = HTMLParser()
     s1 = p.unescape(s)
     print(s1)
     s2 = html.unescape(s)
     print(s2)


class C2TestCase(TestCase):
    def test(self):
        do_html_xml()

if __name__ == "__main__":
    main()
