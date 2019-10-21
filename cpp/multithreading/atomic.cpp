#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <sstream>

using namespace std;

const static int N = 10;
static atomic<bool> ready(false);
static atomic_flag flag = ATOMIC_FLAG_INIT;
static atomic<int> count(0);

static int i_count;
static stringstream stream;

struct Node {
	int val;
	Node *next;
};

inline void mb()
{
	asm volatile ("":::"memory");
}

static atomic<Node *> list_head(nullptr);
static mutex head_lock;

void fn(int id)
{
	while (!ready)
		this_thread::yield();

	if (!flag.test_and_set())
		cout << "thread " << id << " win!" << endl;
}

void fn0(int id)
{
	while (!ready)
		this_thread::yield();

	/* store(), load() */
	if (id == N - 1) {
		count.store(10);
		cout << "Store count to " << count << endl;
	} else {
		while (count.load() == 0);
		cout << "leave loop: " << id << endl;
	}

	int a0 = static_cast<int>(count);
}

void fn1(int id)
{
	while (!ready)
		this_thread::yield();

	if (count.exchange(10) == 0)
		cout << id << " win!" << endl;
}

void fn2(int id)
{
	Node *old_head = list_head;
	Node *node = new Node {id, old_head};

	/* compare_exchange_weak(), compare_exchange_strong() */
	while (!list_head.compare_exchange_weak(old_head, node))
		node->next = old_head;
}

void fn3(int id)
{
	Node *node = new Node { id, nullptr };
	lock_guard<mutex> lock_list_head(head_lock);
	node->next = list_head;
	list_head = node;
}

void fn4(int id)
{
	while (!ready)
		this_thread::yield();

 	/* fetch_add(), fetch_sub(), fetch_and(), fetch_or(), fetch_xor() */
	if (count.fetch_add(1) < 3)
		cout << id << " gets it!" << endl;
}

void fn5(int id)
{
	while (!ready)
		this_thread::yield();

	/* operator++, operator-- */
	if (count++ < N / 2)
		cout << id << " gets it!" << endl;
}

void fn6(int id)
{
	while (!ready)
		this_thread::yield();

	/* operator++, operator-- */
	if ((count += 1) < N / 2)
		cout << id << " gets it!" << endl;
}

void fn7(int id)
{
	while (!ready)
		this_thread::yield();

	while (flag.test_and_set());
	stream << "flag.test_and_set(): " << id << " gets it!" << endl;
	flag.clear();
}

int main()
{
	vector<thread> threads(N);

	for (int i = 0; i < N; i++)
		threads[i] = thread(fn7, i);

	ready = true;

	for (int i = 0; i < N; i++)
		threads[i].join();

	Node *node = list_head;
	while (node) {
		cout << node->val << endl;
		node = node->next;
	}

	atomic<int> a0(0);
	atomic<double> a1(0.0);
	atomic<int *> a2(nullptr);
	cout << "int is_lock_free(): " << a0.is_lock_free() << endl;
	cout << "double is_lock_free(): " << a1.is_lock_free() << endl;
	cout << "int* is_lock_free(): " << a2.is_lock_free() << endl;

	struct A {
		char c0;
		char c1;
		char c2;
		char c3;
		char c4;
		char c5;
		char c6;
		char c7;
	};

	struct B {
	};

	cout << "struct A is_lock_free(): " << atomic<A> { }.is_lock_free() << endl;
	cout << "struct B is_lock_free(): " << atomic<B> { }.is_lock_free() << endl;

	cout << stream.str();
	return 0;
}
