#include <iostream>
using namespace std;

class A2 {
public:
	A2() { cout << "A2()" << endl; }
	~A2() { cout << "~A2()" << endl; }
	virtual void show() { cout << "A2.show(): a2@" << &a2 << endl; }
	virtual void show1() { cout << "A2.show1()" << endl; }

private:
	int a2;
};

class A1 {
public:
	A1() { cout << "A1()" << endl; }
	~A1() { cout << "~A1()" << endl; }
	virtual void show() { cout << "A1.show(): a1@" << &a1 << endl; }
	virtual void show1() { cout << "A1.show1()" << endl; }

private:
	int a1;
	int a2;
};

class A : public A1, public A2
{
	public:
		A() : m_x(0) { }
		~A() { cout << "~A()" << endl; }

	public:
		static int nr;
		const static int nr0 = 1;
		static ptrdiff_t member_offset(const A &a)
		{
			const char *p = reinterpret_cast<const char*>(&a);
			const char *q = reinterpret_cast<const char*>(&a.m_x);
			cout << "A(): " << (void *)p << " " << (void *)q << endl;

			return q - p;
		}
		void show() override {
			cout << "A.show(): m_x @" << &m_x << endl;
			A1::show();
			A2::show();
		}

		void show1() override {
			cout << "m_x@" << &m_x << " " << "m_a@" << (void *)&m_a <<
				" " << "m0@" << &m0 << endl; }

	private:
		int m_x;
		char m_a;
		int m0;
};

int A::nr = 0;

class B : public A
{
	public:
		B() : m_x('a') { }

	public:
		static int m_n;

	public:
		static ptrdiff_t member_offset(const B &b)
		{
			const char *p = reinterpret_cast<const char*>(&b);
			const char *q = reinterpret_cast<const char*>(&b.m_x);
			cout << "B(): " << (void *)p << " " << (void *)q << endl;

			return q - p;
		}

	private:
		char m_x;
};

int B::m_n = 1;

class C
{
	public:
		C() : m_x(0) { }
		virtual ~C() { }

	public:
		static ptrdiff_t member_offset(const C &c)
		{
			const char *p = reinterpret_cast<const char*>(&c);
			const char *q = reinterpret_cast<const char*>(&c.m_x);
			cout << "C(): " << (void *)p << " " << (void *)q << endl;

			return q - p;
		}

	private:
		int m_x;
};

class D
{
public:
	const static int d = 123;
	D() { cout << "D() " << endl; }
	virtual ~D() { cout << "~D() " << endl; }
private:
};

int main()
{
	A a;
	A1 *a1p = &a;
	A2 *a2p = &a;

	cout << &a << endl;
	cout << a1p << endl;
	cout << a2p << endl;

	cout << (a1p == &a) << endl;
	cout << (a2p == &a) << endl;
	cout << (reinterpret_cast<void *>(a1p) == reinterpret_cast<void *>(a2p)) << endl;

	return 0;
}
