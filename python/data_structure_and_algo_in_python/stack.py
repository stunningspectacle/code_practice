#!/usr/bin/python3

class Stack:
    _items = []
    def push(self, item):
            self._items.append(item)
    def pop(self):
        try:
            return self._items.pop(-1)
        except Exception as e:
            raise e
    def size(self):
        return len(self._items)

if __name__ == '__main__':
    s0 = Stack();

    print(s0.size())
    s0.push(100)
    s0.push('aaa')
    s0.push('bbb')

    try:
        print(s0.pop())
        print(s0.pop())
        print(s0.pop())

        print(s0.pop())
    except Exception as e:
        print('Error:', e)

    print('Done')

