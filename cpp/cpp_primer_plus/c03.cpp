#include <iostream>
using namespace std;

const int BUFSIZE = 100;

void main6()
{
	//float f0 = 3.14159e6; // works
	float f0 = 3.14159e7;
	float f1 = f0 + 1;

	cout << "f1 - f0 = " << f1 - f0 << endl;
}

void main5()
{
	float f0 = 10.0 / 3.0;
	long double d0 = 10.0 / 3.0;
	const float million = 1.0e6;

	cout << f0 * million << endl;
	cout << d0 * million << endl;

	cout.setf(ios_base::fixed, ios_base::floatfield);
	cout << f0 * million << endl;
	cout << d0 * million << endl;

	cout << 3.3e-06 << endl;
}

void main4()
{
	cout << BUFSIZE << endl;
	cout << "BUFSIZ = " << BUFSIZ << endl;
	//BUFSIZE = 20;
}

void main3()
{
	bool yes = true;
	bool no = false;

	cout << yes << endl;
	cout << no << endl;

	yes = 100;
	no = 90;

	cout << yes << endl;
	cout << no << endl;

	yes = 0;
	no = -0;

	cout << yes << endl;
	cout << no << endl;
}

void main2()
{
	char c = 'K';
	int i = c;

	cout << "char '" << c << "''s value is "<< i << endl;

}

void main1()
{
	cout << 100 << endl;
	cout << hex << "0x" << 100 << endl;
	cout << oct << "0" << 100 << endl;
	cout << dec << 100 << endl;
}

void main0()
{
	cout << "sizeof(char): " << sizeof(char) << endl;
	cout << "sizeof(int): " << sizeof(int) << endl;
	cout << "sizeof(short): " << sizeof(short) << endl;
	cout << "sizeof(long): " << sizeof(long) << endl;
	cout << "sizeof(long long): " << sizeof(long long) << endl;
	cout << "sizeof(float): " << sizeof(float) << endl;
	cout << "sizeof(double): " << sizeof(double) << endl;
	cout << "sizeof(long double): " << sizeof(long double) << endl;
	cout << "sizeof(void *): " << sizeof(void *) << endl;
}

int main()
{
	main0();
	main1();
	main2();
	main3();
	main4();
	main5();
	main6();

	return 0;
}
