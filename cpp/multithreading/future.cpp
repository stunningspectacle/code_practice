#include <iostream>
#include <thread>
#include <vector>
#include <future>

using namespace std;

int get_num()
{
	//this_thread::sleep_for(chrono::seconds(1));

	int i = rand();
	cout << "generate " << i << endl;

	return i;
}

int main()
{
	future<int> fut = async(get_num);

	this_thread::sleep_for(chrono::seconds(1));
	cout << "try to get num" << endl;
	int num = fut.get();
	cout << "get num " << num << endl;

}

