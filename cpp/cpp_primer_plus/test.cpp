#include <iostream>
using namespace std;

class A
{
	public:
		A() : m_i(0) { }

	protected:
		char m_i;
};

class B
{
	public:
		B() : m_d(0.0) { }

	protected:
		char m_d;
};

class D
{
	public:
		D() : m_i(0) { }

	protected:
		char m_i;
};

class E
{
	public:
		E() : m_i(0) { }

	protected:
		char m_i;
};

class F
{
	public:
		F() : m_i(0) { }

	protected:
		char m_i;
};

class C : public A , public B, public D, public E, public F
{
	public:
		C() : m_c('a') { }

	private:
		char m_c;
};

int main()
{
	C c;
	A *pa = &c;
	B *pb = &c;
	D *pd = &c;
	E *pe = &c;
	F *pf = &c;

	const int x = (pa == &c) ? 1 : 2;
	const int y = (pb == &c) ? 3 : 4;
	const int z = (reinterpret_cast<char*>(pa) == reinterpret_cast<char*>(pb)) ? 5 : 6;
	cout << "pa: " << pa << endl;
	cout << "pb: " << pb << endl;
	cout << "pd: " << pd << endl;
	cout << "pe: " << pe << endl;
	cout << "pf: " << pf << endl;
	cout << (pa == &c) << endl;
	cout << (pb == &c) << endl;
	cout << (pd == &c) << endl;
	cout << (pe == &c) << endl;
	cout << (pf == &c) << endl;
	cout << "&c: " << &c << endl;


	char *c0 = reinterpret_cast<char *>(pa);
	char *c1 = reinterpret_cast<char *>(pb);
	char *c2 = 0;

	cout << "reinterpret_cast<>(pa): " << (void *)(reinterpret_cast<char *>(pa)) << endl;
	cout << "reinterpret_cast<>(pb): " << (void *)(reinterpret_cast<char *>(pb)) << endl;

	cout << "c0: " << (void *)c0 << endl;
	cout << "c1: " << (void *)c1 << endl;
	cout << "c2: " << (void *)c2 << endl;

	std::cout << x << y << z << std::endl;

	return 0;
}
