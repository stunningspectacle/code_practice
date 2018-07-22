#include <iostream>
using namespace std;

template <typename Type>
void myswap(Type &a, Type &b)
{
	Type c = a;
	a = b;
	b = c;
}

template <> void myswap(int &a, int &b);

void func(int a, int b = 10, int c = 20);

void func(int a, int b, int c)
{
	cout << a << " " << b << " " << c << " " <<endl;
}

int main(void)
{
	int a = 10, b = 20;
	cout << "a = " << a << ", b = " << b << endl;
	myswap(a, b);
	cout << "a = " << a << ", b = " << b << endl;

	double c = 50.0, d = 60.0;
	cout << "c = " << c << ", d = " << d << endl;
	myswap(c, d);
	cout << "c = " << c << ", d = " << d << endl;

	return 0;
}

template <> void myswap(int &a, int &b)
{
}

