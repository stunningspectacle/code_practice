#include <iostream>
#include <exception>
#include <typeinfo> // if (typeid(A) == typeid(*ptr))
using std::cin;
using std::cout;
using std::endl;

class A
{
	int a;
public:
	virtual void sayA() const { cout << "a:" << a << endl; }
};

class B: public A
{
	int b;
public:
	virtual void sayB() const { cout << "b:" << b << endl; }
};

class C: public B
{
	int c;
public:
	virtual void sayC() const { cout << "c:" << c << endl; }
};

int main()
{
	A a0;
	B b0;
	C c0;

	A *p[3];

	p[0] = &a0;
	p[1] = &b0;
	p[2] = &c0;

	for (int i = 0; i < 3; i++) {
		p[i]->sayA();
		B *bp;
		if (bp = dynamic_cast<B *>(p[i]))
			bp->sayB();
		C *cp;
		if (cp = dynamic_cast<C *>(p[i]))
			cp->sayC();

		try {
			B &rb = dynamic_cast<B &>(*p[i]);
			rb.sayB();
			C &rc = dynamic_cast<C &>(*p[i]);
			rc.sayC();
		}
		catch (std::bad_cast &bc)
		{
			cout << "it's really bad cast " << endl;
			bc.what();
		}

		if (typeid(C) == typeid(*p[i]))
			cout << "yes, it's a class C" << endl;
		else
			cout << "no, it's not a class C" << endl;
	}
	
	return 0;
}

