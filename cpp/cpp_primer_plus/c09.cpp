
#include <iostream>
#include <cstring>
#include <new>
//using namespace std;
using std::cout;
using std::endl;
using std::ios_base;

namespace Mike
{

int mike = 100.0;
void hello()
{
	cout << "this is Mike's hello" << endl;
}

}


float warning = 1.0;

char buff1[128];
char buff2[256]; 

void main2()
{
	cout << "buff1 @" << (void *)buff1 << " to " <<
		(void *)(buff1 + sizeof(buff1)) << endl;
	cout << "buff2 @" << (void *)buff2 << " to " <<
		(void *)(buff2 + sizeof(buff2)) << endl;
	cout << "warning @" << &warning << endl;

	int *nums = new (buff1) int[10];
	cout << "nums @" << nums << endl;

	Mike::hello();
}

void main1()
{
	float warning = 0.1;

	cout << warning << endl;
	//ios_base::fmtflags flags = cout.setf(ios_base::showpoint);
	cout.setf(ios_base::showpoint);
	cout << ::warning << endl;
	cout.unsetf(ios_base::showpoint);
	//cout.setf(flags);
	cout << ::warning << endl;
}

int main()
{
	main2();

	return 0;
}

