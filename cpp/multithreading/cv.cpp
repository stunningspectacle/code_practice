#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>

using namespace std;

class Base
{
public:
	static const int N = 10;
	Base(void (*f)(int id)) {
		for (int i = 0; i < N; i++)
			threads.push_back(thread(f, i));
	}
	~Base() {
		for (auto &t : threads)
			t.join();
	}

private:
	vector<thread> threads;
};

static mutex mtx;
condition_variable cv;
static bool ready = false;
static atomic<int> threads_done(0);

static condition_variable cv_producer;
static condition_variable cv_consumer;
static int product;

void fn0(int id)
{
	unique_lock<mutex> lck(mtx);
	cout << "thread " << id << " start " << endl;
 	/* if the func return false, won't wake up;
	 * the function will be exec before wait, if return true, won't sleep
	 */
	cv.wait(lck, []{
			return ready;
			});

	cout << "thread " << id << " waken up" << endl;
	++threads_done;
	if (threads_done == Base::N)
		cv.notify_one();
}

void fn1(int id)
{
	unique_lock<mutex> lck(mtx);
	if (!cv.wait_for(lck, chrono::milliseconds(id * 1000), []{ return ready; }))
		cout << "timeout happens on " << id << endl;
	else
		cout << id << " waken up" << endl;
}

void go()
{
	ready = true;

	/* cv.notify_all(), cv.notify_one() */
	cv.notify_all();
}

void producer(int id)
{
	while (1) {
		unique_lock<mutex> lck(mtx);
		cv_producer.wait(lck, []{ return product == 0; });
		product = 1;
		cout << "product produced by producer " << id << endl;
		cv_consumer.notify_one();
		this_thread::sleep_for(chrono::milliseconds(500));
	}
}

void consumer(int id)
{
	while (1) {
		unique_lock<mutex> lck(mtx);
		cv_consumer.wait(lck, []{ return product == 1; });
		product = 0;
		cout << "product consumed by consumer " << id << endl;
		cv_producer.notify_one();
		this_thread::sleep_for(chrono::milliseconds(500));
	}
}

void fn2(int id)
{
	this_thread::sleep_for(chrono::milliseconds(500));

	lock_guard<mutex> lck(mtx);
	cout << " this is " << id << endl;
}

int main()
{
	vector<thread *> threads;

	for (int i = 0; i < 10; i++)
		threads.push_back(new thread(fn2, i));


	for (auto &t : threads)
		t->detach();

	cout << "main exit" << endl;

	return 0;
}
