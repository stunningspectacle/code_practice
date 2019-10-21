#include <iostream>
#include <thread>
#include <unistd.h>
#include <memory>
#include <vector>
#include <array>
#include <mutex>
#include <atomic>

using namespace std;

static mutex barrier;

static atomic_flag lock_os = ATOMIC_FLAG_INIT;

void thread_mutex(int id)
{
	int count = 0;

	while (true) {
		{
			lock_guard<mutex> lock_it(barrier);
			cout << __func__ << " from" << " " << id << " " << count++ << endl;
		}
		sleep(1);
	}
}

void thread_atomic_flag(int id)
{
	int count = 0;

	while (true) {
		while (lock_os.test_and_set()) { }
		cout << __func__ << " from" << " " << id << " " << count++ << endl;
		lock_os.clear();
		sleep(1);
	}
}


void thread1(int id)
{
	int count = 0;

	while (true) {
		lock_guard<mutex> lock_it(barrier);
		cout << __func__ << " from" << " " << id << " " << count++ << endl;
		sleep(1);
	}
}

static const int N = 10;
int test0()
{
	vector< unique_ptr<thread> > threads(N);

	for (int i = 0; i < N; i++)
		threads[i] = unique_ptr<thread>(new thread(thread_atomic_flag, i));

	for (auto &t : threads)
		t->join();

	return 0;
}

int test1()
{
	unique_ptr<thread> threads[10];
	for (int i = 0; i < N; i++)
		threads[i] = unique_ptr<thread>(new thread(thread1, i));
	for (int i = 0; i < N; i++)
		threads[i]->join();

	return 0;
}

void test2()
{
	thread threads[N];
	array<thread *, N> tasks;

	for (int i = 0; i < N; i++) {
		threads[i] = thread(thread1, i);
		tasks[i] = new thread(thread1, i);
	}

	for (int i = 0; i < N; i++) {
		threads[i].join();
		tasks[i]->join();
	}
}

int main()
{
	test0();

	return 0;
}
