#include <iostream>
#include <cstring>
using namespace std;

auto func0(int a, int b) -> decltype(a + b)
{
	cout << sizeof(decltype(a + b)) << endl;

	return (a + b);
}

void main7()
{
	func0(10, 20);
}

template <typename T>
void question(T a)
{
	cout << "question(T a) is used" << endl;
}

template <typename T>
void question(T *a)
{
	cout << "question(T *a) is used" << endl;
}

void question(char *a)
{
	cout << "question(char *a) is used" << endl;
}

void main6()
{
	char a;

	question(&a); // compiler to select
	question<>(&a); // Use the best template one
	question<char *>(&a); // replace typename by char * in template
}

struct job
{
	string name;
	float value0;
	float value1;
};

template <typename T>
void myswap(T & a, T & b)
{
	T temp;
	temp = a;
	a = b;
	b = temp;
}
template void myswap<int>(int &a, int &b); // explicit instantiation 

//template <> void myswap<job>(job & a, job & b)
template <> void myswap(job &a, job &b) // explicit specialization
{
	float v = a.value0;
	a.value0 = b.value0;
	b.value0 = v;

	v = a.value1;
	a.value1 = b.value1;
	b.value1 = v;
}


template <typename T>
void myswap(T *a, T *b, int size)
{
	T temp;

	for (int i = 0; i < size; i++) {
		temp = a[i];
		a[i] = b[i];
		b[i] = temp;
	}
}

void main5()
{
	int a = 10, b = 20;
	cout << a << ", " << b << endl;
	myswap(a, b);
	cout << a << ", " << b << endl;

	double c0 = 10.12, d0 = 20.23;
	cout << c0 << ", " << d0 << endl;
	myswap(c0, d0);
	cout << c0 << ", " << d0 << endl;

	string c = "abc", d = "def";
	cout << c << ", " << d << endl;
	myswap(c, d);
	cout << c << ", " << d << endl;

	char s0[] = "12345";
	char s1[] = "abcde";

	cout << s0 << ", " << s1 << endl;
	myswap(s0, s1, sizeof(s0));
	cout << s0 << ", " << s1 << endl;

	job j0 = { "look", 100, 200 };
	job j1 = { "see", 1000, 3000 };

	cout << j0.name << ": " << j0.value0 << ", " << j0.value1 << endl;
	cout << j1.name << ": " << j1.value0 << ", " << j1.value1 << endl;
	myswap(j0, j1);
	cout << j0.name << ": " << j0.value0 << ", " << j0.value1 << endl;
	cout << j1.name << ": " << j1.value0 << ", " << j1.value1 << endl;
}

const char * left(const char *s, int nr = 1)
{
	int i = 0;
	char *ps;

	cout << "Using string version" << endl;
	if (nr < 0)
		nr = 0;

	nr = strlen(s) > nr ? nr : strlen(s);
	ps = new char [nr + 1];
	while (i < nr && s[i]) {
		ps[i] = s[i];
		i++;
	}
	ps[i] = '\0';

	return ps;
}

const char * left(unsigned int code, int nr = 1)
{
	int nums = 0;
	char digits[20] = {0};
	char *ps;
	int i = 0;

	cout << "Using int version" << endl;

	while (code) {
		digits[nums++] = (code % 10) + '0';
		code /= 10;
	}

	ps = new char [nr + 1];
	while (--nums >= 0)
		ps[i++] = digits[nums];

	ps[nr] = '\0';

	return ps;
}

void main4()
{
	unsigned int num;
	const char *ps;

	cout << "Input a number" << endl;
	cin >> num;

	ps = left(num, 4);
	cout << ps << endl;
	delete [] ps;

	ps = left(num);
	cout << ps << endl;
	delete [] ps;
}

void main3()
{
	char s[32];
	const char *ps;

	cout << "Input a string" << endl;
	cin.get(s, sizeof(s));

	ps = left(s, 4);
	cout << ps << endl;
	delete [] ps;

	ps = left(s);
	cout << ps << endl;
	delete [] ps;
}

struct student
{
	string name;
	string tag;
	int used;
};

const student & use(student & s)
{
	s.used++;

	return s;
}

void main2()
{
	student s0 = { "Leo", "good", 0 };
	student s1;

	getline(cin, s0.tag);
	cout << "used: " << s0.used << endl;
	const student & s2 = use(s0);
	cout << "used: " << s0.used << endl;
}

void ref0(const int & num)
{
	int b = 0;

	b *= num;
}

void main1()
{
	int i0 = 100;
	int & i1 = i0;

	cout << i0 << ", " << i1 << endl;
	i1++;
	cout << i0 << ", " << i1 << endl;

	ref0(i0);
	cout << i0 << ", " << i1 << endl;

	ref0(10);
	cout << i0 << ", " << i1 << endl;

}

int main()
{
	main7();

	return 0;
}
