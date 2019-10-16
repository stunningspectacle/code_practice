#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <vector>
#include <algorithm>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::auto_ptr;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::for_each;

class Report
{
	string name;
public:
	static int count;
	Report(const string &s = "none"): name(s)
	{
		cout << "create " << name << count++ << endl;
	}
	~Report() { cout << "delete " << name << count-- << endl; }
	void show() const { cout << "this is " << name << count << endl; }
};
int Report::count = 0;

void test(Report &t)
{
	t.show();
}

unique_ptr<Report> test0(Report &t)
{
	unique_ptr<Report> r0(new Report("from test0"));

	return r0;
}

unique_ptr<int> make_int(int n)
{
	return unique_ptr<int>(new int(n));
}

void show(unique_ptr<int> &np)
{
	cout << *np << " ";
}

void main2()
{
	vector<unique_ptr<int> > vp(20);

	cout << "vp.size():" << vp.size() << endl;

	for (int i = 0; i < vp.size(); i++)
	{
		vp[i] = make_int(rand() % 1000);
		cout << *vp[i] << " ";
	}
	cout << endl;

	vp.push_back(make_int(rand() % 1000));
	cout << "vp.size():" << vp.size() << endl;

	for_each(vp.begin(), vp.end(), show);
	cout << endl;
}

void main1()
{
	cout << "A demo of smart pointer:" << endl;
	{
		auto_ptr<Report> p(new Report("auto_ptr"));
		p->show();
	}
	{
		shared_ptr<Report> p(new Report("shared_ptr"));
		p->show();
		shared_ptr<Report> p0;
		p0 = p;
	}
	{
		unique_ptr<Report> p(new Report("unique_ptr"));
		p->show();
		test(*p);
	}

	{
		unique_ptr<char> c(new char [10]);
		//strncpy(&c[0], "hello", 10);
	}

	{
		/* Use unique_ptr to new object array, shared_ptr is not allowed */
		unique_ptr<Report []> reports(new Report[5]);

		unique_ptr<double> dp(new double);
	}

	{
		unique_ptr<Report> r0(new Report("jack"));
		unique_ptr<Report> r1;

		r1 = test0(*r0);
		cout << "I got from test0" << endl;
	}
}

int main()
{
	main2();

	return 0;
}
