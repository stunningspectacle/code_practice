#include <iostream>
#include <thread>
using namespace std;

typedef void (*Myfunc)(void);

int main()
{
	cout << "sleep 1000*1000*1000 nanoseconds" << endl;
	this_thread::sleep_for(chrono::nanoseconds(1000 * 1000 * 1000));
	cout << "sleep 1000 millisecond" << endl;
	this_thread::sleep_for(chrono::milliseconds(1000));
	cout << "sleep 1000 * 1000 microsecond" << endl;
	this_thread::sleep_for(chrono::microseconds(1000 * 1000));
	cout << "sleep 1 second" << endl;
	this_thread::sleep_for(chrono::seconds(1));
	cout << "sleep done" << endl;

	return 0;
}
