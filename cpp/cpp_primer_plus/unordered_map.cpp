#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <map>
#include <algorithm>
#include <memory>

using namespace std;

template<typename M>
void showM(M &m)
{
	for (auto &item : m)
		cout << item.first << ":" << item.second << " ";
	cout << endl;

}

template<typename T>
void show(T &t)
{
	for (auto &item : t)
		cout << item << " ";
	cout << endl;
}

vector<string> test_set(const vector<string> &s1, const vector<string>&s2)
{
	set<string> set0;

	for (auto &s : s1)
		set0.emplace(s);
	for (auto &s : s2)
		set0.emplace(s);

	vector<string> v0;
	for (auto &s: set0)
		v0.push_back(s);

	return v0;
}

void test_vector()
{
	vector<int> v0;

	v0.resize(10);
	v0.push_back(222);
	show(v0);

	cout << v0.size() << endl;
}

void test_map()
{
	vector<int> nums = {33, 44, 88, 77, 22, 22, 33};

	vector< pair<int, int> > v0;
	for (int i = 0; i < nums.size(); i++)
		v0.push_back(pair<int, int>(nums[i], i));

	showM(v0);
	sort(v0.begin(), v0.end());
	showM(v0);

}

std::pair<int, int> findTwoSum(const std::vector<int>& list, int sum)
{
	vector< pair<int, int> >v0;
	for (int i = 0; i < list.size(); i++)
		v0.push_back(pair<int, int>(list[i], i));

	sort(v0.begin(), v0.end());

	showM(v0);
	auto begin = v0.begin();
	auto end = --v0.end();
	while (begin != end) {
		cout << (*begin).first << " " << (*end).first << endl;
		if ((*begin).first + (*end).first == sum)
			return pair<int, int>((*begin).second, (*end).second);
		if ((*begin).first + (*end).first > sum)
			end--;
		else
			begin++;
	}
	return pair<int, int>(-1, -1);
}

class Base {
public:
	void f() {
		cout<<"Base\n";
	}
};

class Derived:public Base {
public:
	void f() {
		cout<<"Derived\n";
	}
};

int main1() { 
	Derived obj; 
	obj.f();
	obj.Derived::f();
	obj.Base::f();

	return 0;
}

class abc { 
	public: 
		int i; 

		abc(int i) { 
			cout << i << endl;
			cout << this->i << endl;
			i = i;
		}
};


class A {
	int Avalue;
public:
	A(int n): Avalue(n) { cout << "A()" << Avalue << endl; } 
	~A() { cout << "~A()" << Avalue << endl; }
	void show() { cout << "A::show():" << Avalue << endl; }
};

class B : public A {
	int Bvalue;
public:
	B(int n): A(n+1) { cout << "B()" << Bvalue << endl; } 
	~B() { cout << "~B()" << Bvalue << endl; }
	void show() { cout << "B::show():" << Bvalue << endl; }
};

class C : public A {
	int Cvalue;
public:
	C(int n): Cvalue(n), A(n+1) { cout << "C()" << Cvalue << endl; } 
	~C() { cout << "~C()" << Cvalue << endl; }
	void show() { cout << "C::show():" << Cvalue << endl; }
};

class D : public B, public C {
	int Dvalue;
public:
	D(int n = 1000 ): Dvalue(n), B(n), C(2 * n){ cout << "D()" << Dvalue << endl; } 
	~D() { cout << "~D()" << Dvalue << endl; }
};

int main()
{
	D d;

	d.B::show();
	d.C::show();

	return 0;
}
