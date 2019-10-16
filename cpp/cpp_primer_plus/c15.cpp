#include <iostream>
#include <string>
#include <cstdlib>
#include <exception>
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::cin;

class TV
{
	int state;

public:
	friend class Remote;
	enum { OFF, ON };
	TV(): state(0) { };
	void show() const { cout << "state: " << state << endl; };
};

class Remote
{
public:
	void onoff(TV &tv) { tv.state = (tv.state == (TV::ON) ? TV::OFF : TV::ON); };
};

template <typename Item>
class Queue
{
	const int qsize;
	int items;
	class Node
	{
	public:
		Item item;
		Node *next;
		Node(Item t): item(t), next(nullptr) { }
	};
	Node *front;
	Node *rear;
public:
	enum { MAX_SIZE = 10 };
	Queue(int qs = MAX_SIZE): qsize(qs), front(nullptr), rear(nullptr), items(0) { }
	bool isempty() const { return items == 0; }
	bool isfull() const { return items == qsize; }
	bool enqueue(Item item);
	bool dequeue(Item &item);
	void show() const;
	//friend ostream & operator<<(ostream &os, const T &);
};

template <typename Item>
bool Queue<Item>::enqueue(Item item)
{
	if (isfull())
		return false;
	Node *n = new Node(item);
	if (items == 0)
		front = rear = n;
	else
		rear->next = n;
	items++;

	return true;
}

/*
ostream & operator<<(ostream &os, const T &t)
{
	Queue<Item>::Node *n;
	n = t.front;

	while (n) {
		os << n->item << "-->";
		n = n->next;
	}
}
*/

template <typename Item>
void Queue<Item>::show() const
{
	Node *n = front;

	while (n) {
		cout << n->item << "-->";
		n = n->next;
	}
	cout << "nullptr" << endl;
}

template <typename Item>
bool Queue<Item>::dequeue(Item &item)
{

	if (isempty())
		return false;
	Node *tmp = front;
	item = front->item;
	front = front->next;
	items--;

	delete tmp;

	return true;
}

class E1
{
	string msg;
public:
	E1(const string &s = "this is E1"): msg(s) { }
	void say() { cout << msg << endl; }
	~E1() { cout << msg << "~E1() called" << endl; }
};

class E2
{
	string msg;
public:
	E2(const string &s = "this is E2"): msg(s) { }
	void say() { cout << msg << endl; }
};

void test1()
{
	throw E1();
}

void test2()
{
	throw E2();
}

void test3()
{
	throw "This is const string";
}

void test4()
{
	throw 10;
}

void middle1()
{
	E1 e("e1 from middle1()");
	test1();

	cout << "this is middle1()" << endl;
}

void my_unexpected()
{
	throw E1();
	try
	{
		cout << "something bad happens, let's ignore it" << endl;
	}
	catch (...) 
	{
		cout << "my_unexpected()" << endl;
	}
}

int main()
{
	TV t0;
	Remote r0;

	t0.show();
	r0.onoff(t0);
	t0.show();

	Queue<int> q0(8);
	q0.show();

	q0.enqueue(100);
	q0.enqueue(200);
	q0.show();

	int a;
	q0.dequeue(a);
	q0.show();

	std::set_unexpected(my_unexpected);
	std::set_terminate(my_unexpected);
	while (1)
	{
		int n;

		cin >> n;

		try {
			switch(n)
			{
				case 1:
					middle1();
					break;
				case 2:
					test2();
					break;
				case 3:
					test3();
					break;
				case 4:
					test4();
					break;
				default:
					exit(0);
			}
		}

		catch (const char *s) {
			cout << s << endl;
		}
		catch (E1 &e1) {
			e1.say();
		}
		catch (E2 &e2) {
			e2.say();
		}
		/*
		catch (...) {
			cout << "something bad happens" << endl;
		}
		*/
	}

}

