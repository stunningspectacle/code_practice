#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <memory>
#include <cctype>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <list>

using std::cin;
using std::cout;
using std::endl;
using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::string;
using std::array;
using std::iterator;
using std::ofstream;
using std::ifstream;
using std::for_each;
using std::random_shuffle;
using std::sort;
using std::copy;
using std::ostream_iterator;
using std::insert_iterator;

class Report
{
	string name;
	static int count;
public:
	Report(const string &n = "none"): name(n)
	{
		name += '0' + count;
		cout << "create " << name << endl;
		count++;
	}
	~Report()
	{
		cout << "destroy " << name << endl;
		count--;
	}
	void show() const { cout << "this is " << name << endl; }
};

int Report::count = 0;

enum { NR = 2 };

template <typename VEC>
void dump_vec(VEC &v)
{
	for (auto at = v.begin(); at != v.end(); at++)
	//for (auto at: v) {
	if (at == v.end() - 1)
		cout << *at << endl;
	else
		cout << *at << ",";
}

template void dump_vec<vector<double> >(vector<double> &v);

template <> void dump_vec(vector<int> &v)
{
	for (auto at = v.begin(); at != v.end(); at++) {
	//for (auto at: v) {
		if (at == v.end() - 1)
			cout << *at << endl;
		else
			cout << *at << ";";
	}
}

bool compare_int(const int &a, const int &b)
{
	return (a > b);
}

void main6()
{
	vector< shared_ptr<Report> > reports(5);
	for (auto &r: reports)
		r = shared_ptr<Report>(new Report("good"));
	for (auto &r: reports)
		r->show();
	cout << "end of " << __func__ << endl;

	vector< shared_ptr<Report> > reports0(5);
	copy(reports.begin(), reports.end(), reports0.begin());
	for (auto &r: reports)
		r->show();
}

void main5()
{
	string s0[] = { "aaa", "bbb", "ccc", "ddd" };

	vector<string> v0(2);
	copy(s0, s0 + 4, v0.begin());
	dump_vec(v0);

	copy(s0, s0 + 4, v0.begin());
	dump_vec(v0);

	insert_iterator< vector<string> > insert_it(v0, v0.begin());
	copy(s0, s0 + 4, insert_it);
	dump_vec(v0);

	std::back_insert_iterator< vector<string> > back_ins_it(v0);
	copy(s0, s0 + 2, back_ins_it);
	dump_vec(v0);

	std::list<string> l0(0);
	std::front_insert_iterator< std::list<string> > front_ins_it(l0);
	copy(s0 + 2, s0 + 4, front_ins_it);
	for (auto &a: l0)
		cout << a << endl;
}

void main4()
{
	vector<int> v0 = { 4, 8, 12, 9, 10, 15, 7, 6 };
	dump_vec(v0);

	ostream_iterator<int, char> os_it(cout, "!");
	copy(v0.begin(), v0.end(), os_it);
	cout << endl;
	copy(v0.rbegin(), v0.rend(), os_it); // use reverse iterator
	cout << endl;

	vector<int>::reverse_iterator rit;
	for (rit = v0.rbegin(); rit != v0.rend(); rit++)
		cout << *rit << "!";
	cout << endl;
}

void main3()
{
	ifstream ifs("randint.txt");
	vector<int> v0;

	cout << v0.size() << endl;
	if (!ifs.is_open()) {
		cout << "failed to open file" << endl;	
		return;
	}

	int tmp;
	ifs >> tmp;
	while (!ifs.fail() && !ifs.eof())
	{
		v0.push_back(tmp);
		ifs >> tmp;
	}

	ifs.close();

	cout << "size:" << v0.size() << endl;
	dump_vec(v0);

	cout << "v0[999]:" << v0[999] << endl;
	cout << "v0[1000]:" << v0[1000] << endl;
	cout << "v0[1001]:" << v0[1001] << endl;
	cout << "capacity:" << v0.capacity() << endl;
	cout << "v0[1023]:" << v0[1023] << endl;
	cout << "v0[1024]:" << v0[1024] << endl;
	cout << "v0[1025]:" << v0[1025] << endl;
	cout << "v0[1026]:" << v0[1026] << endl;

	v0.erase(v0.begin() + 10, v0.end());
	dump_vec(v0);
	cout << "size:" << v0.size() << endl;

	vector<int> v1 = { 123, 345 };

	v0.insert(v0.begin(), v1.begin(), v1.end());
	dump_vec(v0);
	cout << "size:" << v0.size() << endl;
	v0.insert(v0.end(), v1.begin(), v1.end());
	dump_vec(v0);
	cout << "size:" << v0.size() << endl;

	vector<double> v2 = { 3.3, 4.4 };
	dump_vec(v2);

	v0.swap(v1);
	dump_vec(v0);

	v0.swap(v1);
	dump_vec(v0);

	random_shuffle(v0.begin(), v0.end());
	dump_vec(v0);

	random_shuffle(v0.begin(), v0.end());
	dump_vec(v0);

	sort(v0.begin(), v0.end());
	dump_vec(v0);
	sort(v0.begin(), v0.end(), compare_int);
	dump_vec(v0);

	double arrs[] = { 2.4, 3.5, 6.7, 22.22 };
	for (auto &a: arrs)
		++a;

	for (auto a: arrs)
		cout << a << endl;
}

void main2()
{
	ofstream ofs;

	ofs.open("randint.txt");
	if (!ofs.is_open()) {
		cout << "failed to open file" << endl;	
		return;
	}

	for (int i = 0; i < 50; i++)
		ofs << rand() % 10000 << " ";
	ofs.close();
}

void main1()
{
	vector<string> names(NR);
	array<double, NR> scores;
	char c;

	cout << names.size() << endl;
	for (int i = 0; i < NR; i++)
	{
		cout << "Name: ";
		getline(cin, names[i]);
		cout << "Score: ";
		cin >> scores[i];
		while (cin.get(c) && c != '\n');
	}

	int i = 0;
	for (auto at = names.begin(); at != names.end(); at++)
		cout << *at << ":" << scores[i++] << " ";
	cout << endl;
}

int main()
{
	main6();
	return 0;
}
