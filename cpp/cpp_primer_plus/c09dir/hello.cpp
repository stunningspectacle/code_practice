#include <iostream>
#include "consts.h"
using namespace std;

extern const int dd;

namespace hello_cpp
{
      int hello_count = 1024;
}

void hello()
{
	cout << "hello: " << aa << ", " << bb << ", dd=" << dd << endl;
	cout << "hello_count:" << hello_cpp::hello_count << endl;
}
