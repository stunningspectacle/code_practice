#include <iostream>
#include <cmath>
#include <ctime>
using std::cout;
using std::endl;
using std::ostream;

typedef int Item;

struct Node
{
	Item item;
	struct Node *next;
};

class Queue
{
	enum { Q_SIZE = 10 };
	Node *front;
	Node *rear;
	int items;
	const int qsize;
public:
	Queue(int qs = Q_SIZE);
	~Queue();
	bool enqueue(const Item &item);
	bool dequeue(Item &item);
	bool isfull() const { return items == qsize; }
	bool isempty() const { return items == 0; }

	friend ostream& operator<<(ostream &os, const Queue &q);
};

Queue::Queue(int qs): qsize(qs), front(nullptr), rear(nullptr), items(0)
{
	cout << "This is Queue(int)" << endl;

	//front = rear = nullptr;
	//items = 0;
	//qsize = qs;
}

Queue::~Queue()
{
	cout << "This is ~Queue()" << endl;
	Node *t;

	while (front)
	{
		t = front;
		front = front->next;
		delete t;
	}
}

ostream & operator<<(ostream &os, const Queue &q)
{
	os << q.items << " items: "; 

	Node *n = q.front;
	while (n) {
		os << n->item << "->";
		n = n->next;
	}
	os << "null";

	return os;
}

bool Queue::enqueue(const Item &item)
{
	Node *n;

	if (isfull())
		return false;
	
	n = new Node;

	n->item = item;
	n->next = nullptr;

	if (front == nullptr)
		front = n;
	else
		rear->next = n;

	rear = n;

	items++;

	return true;
}

bool Queue::dequeue(Item &item)
{
	Node *t;

	if (isempty())
		return false;
	item = front->item;
	t = front;
	front = front->next;
	delete t;

	items--;
	if (items == 0)
		rear = nullptr;

	return true;
}

int main()
{
	Item item;

	Queue q0 = Queue(20);
	q0.enqueue(100);
	q0.enqueue(200);
	cout << q0 << endl;

	q0.dequeue(item);
	cout << q0 << endl;

	for (int i = 0; i < 10; i++) {
		cout << rand() << endl;
	}
}

