#include <iostream>
#include "consts.h"
using namespace std;

extern void hello();
extern "C" void hello1(int);
extern const int dd = 100;

int main()
{
	//using namespace hello_cpp;

	cout << "main says:" << aa << ", " << bb << endl;
	hello();
	hello1(100);
	//cout << "hello_count: " << hello_count << endl;
}
