#include <iostream>
using std::cin;
using std::cout;
using std::endl;

class A
{
public:
	void show(int a)
	{
		cout << a << endl;
	}
};

class B: public A
{
public:
	void show(int a)
	{
		A::show(a);
	}

	void show(int a, int b)
	{
		cout << a << b << endl;
	}
};

int main()
{
	B b0;
	b0.show(1);
}

