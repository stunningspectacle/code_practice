#include <iostream>
using std::cout;
using std::cin;
using std::endl;

class A
{
	int a;
};

const int age = 100;
void test(const int *n)
{
	int *p;

	p = const_cast<int *>(n);
	(*p)++;
}

int main()
{
	int a = 100;
	cout << a << ", " << age << endl;

	int *p = const_cast<int *>(&age);

	test(&a);
	cout << a << ", " << age << endl;

	int *x = (int *)age;
	int *y = reinterpret_cast<int *>(0);
	int *z = (int *)0;
	A *ap = (A *)0;

	cout << "x:" <<x << " y:" << y << " z:" << z << " ap:" << ap << endl;

	return 0;
}
