#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <algorithm>
#include <queue>
#include <stack>
//#include <priority_queue>
#include <array>
#include <iterator>
#include <set>
#include <vector>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::ostream;
using std::ofstream;
using std::ifstream;
using std::list;
using std::for_each;
using std::queue;
using std::priority_queue;
using std::stack;
using std::array;
using std::set;
using std::vector;

template <typename C>
void dump_container(C &container)
{
	for (auto &it: container)
		cout << it << " ";
	cout << endl;
}

void outint(int i)
{
	cout << i << " ";
}

bool less_than(int i, int j)
{
	return (i > j); 
}

void main14()
{
	string s0[] = { "buffoon", "thinker", "for", "heavry", "can", "for" };
	string s1[] = { "metal", "any", "food", "elegant", "deliver", "for" };
	const int N = sizeof(s0) / sizeof(s0[0]);

	std::ostream_iterator<string, char> out(cout, " ");
	cout << "set_union:";
	set_union(s0, s0 + N, s1, s1 + N, out);
	cout << endl;

	set<string> set0(s0, s0 + N);
	set<string> set1(s1, s1 + N);

	cout << "set0: ";
	copy(set0.begin(), set0.end(), out);
	cout << endl;
	cout << "set1: ";
	copy(set1.begin(), set1.end(), out);
	cout << endl;

	cout << "set_intersection:";
	set_intersection(set0.begin(), set0.end(), set1.begin(), set1.end(), out);
	cout << endl;

	cout << "set_difference:";
	set_difference(set0.begin(), set0.end(), set1.begin(), set1.end(), out);
	cout << endl;


	string s2[] = { "aaa", "bbb", "ccc" };
	set<string> set2(s2, s2 + 3);
	set_union(set0.begin(), set0.end(), set1.begin(), set1.end(),
			std::insert_iterator< set<string> >(set2, set2.begin()));
	copy(set2.begin(), set2.end(), std::ostream_iterator<string, char>(cout, " "));
	cout << endl;

	set2.insert(string("kkk"));
	set2.insert(string("ghost"));
	set2.insert(string("spook"));
	copy(set2.begin(), set2.end(), std::ostream_iterator<string, char>(cout, " "));
	cout << endl;

	copy(set2.lower_bound("ghost"), set2.upper_bound("spook"), out);
	cout << endl;

	vector<string> v0;
	copy(set2.rbegin(), set2.rend(),
			std::insert_iterator< vector<string> >(v0, v0.begin()));
	copy(v0.begin(), v0.end(), std::ostream_iterator<string, char>(cout, " "));
	cout << endl;
	/*
	vector<int> v0;
	for (int i = 0; i < 50; i++)
		v0.push_back(rand() % 100);
	copy(v0.begin(), v0.end(), std::ostream_iterator<int, char>(cout, " "));
	cout << endl;

	set<int, less_than> set3(v0.begin(), v0.end());
	copy(set3.begin(), set3.end(), std::ostream_iterator<int, char>(cout, " "));
	cout << endl;
	*/
}

void main13()
{
	array<int, 10> a0 = { 0 };
	cout << a0.size() << endl;

	for_each(a0.begin(), a0.end(), outint);
	cout << endl;
	dump_container(a0);
}

void main12()
{
	stack<int> s0;

	for (int i = 0; i < 10; i++)
	{
		int tmp = rand() % 100;
		s0.push(tmp);
		cout << tmp << " ";
	}
	cout << endl;
	for (int i = 0; i < 10; i++)
	{
		cout << s0.top() << " ";
		s0.pop();
	}
	cout << endl;
}

void main11()
{
	priority_queue<int> q0;
	list<int> l0;

	for (int i = 0; i < 50; i++)
		q0.push(rand() % 100);

	for (int i = 0; i < 50; i++)
	{
		cout << q0.top() << " ";
		l0.insert(l0.begin(), q0.top());
		q0.pop();
	}
	cout << endl;

	dump_container(l0);
}

void main10()
{
	queue<int> q0;

	cout << q0.empty() << endl;
	q0.push(rand() % 100);
	q0.push(rand() % 100);

	cout << q0.size() << endl;
	cout << q0.front() << endl;
	q0.pop();
	cout << q0.front() << endl;
	q0.pop();
	cout << q0.size() << endl;
}

void main9()
{
	list<int> one(5, 2);
	int stuff[5] = { 1, 2, 8, 4, 6 };
	list<int> two;
	two.insert(two.begin(), stuff, stuff + 5);
	int more[6] = { 6, 4, 2, 4, 6, 5 };
	list<int> three(two);
	three.insert(three.end(), more, more + 6);

	cout << "list one: ";
	for_each(one.begin(), one.end(), outint);
	cout << endl << "list two: ";
	for_each(three.begin(), three.end(), outint);
	cout << endl << "list three: ";
	for_each(three.begin(), three.end(), outint);
	three.remove(2);
	cout << endl << "list three minus 2s: ";
	for_each(three.begin(), three.end(), outint);
	three.splice(three.begin(), one);
	cout << endl << "list three after splice: ";
	for_each(three.begin(), three.end(), outint);
	cout << endl << "list one after splice: ";
	for_each(one.begin(), one.end(), outint);
	three.unique();
	cout << endl << "list three after unique: ";
	for_each(three.begin(), three.end(), outint);
	three.sort();
	three.unique();
	cout << endl << "list three after sort & unique: ";
	for_each(three.begin(), three.end(), outint);
	two.sort();
	cout << endl << "list two after sort: ";
	for_each(two.begin(), two.end(), outint);
	three.merge(two);
	cout << endl << "Sorted two merged into three: ";
	for_each(three.begin(), three.end(), outint);
	cout << endl;
}

void main8()
{
	list<string> l0(5, "This is a test");
	dump_container(l0);
	string s0("s0");
	l0.insert(l0.begin(), s0);
	dump_container(l0);
	
	string s1("s1");
	l0.insert(l0.end(), s1);
	dump_container(l0);

	list<string>::iterator it;
	it = l0.begin();
	l0.erase(it);
	dump_container(l0);
} 
void main7()
{
	cout.setf(std::ios::hex, std::ios::basefield);
	cout << string::npos << endl;
	cout.setf(std::ios::oct, std::ios::basefield);

	string s0;

	cout << s0.size() << endl;
	cout << s0.capacity() << endl;

	s0.reserve(100);
	cout << s0.size() << endl;
	cout << s0.capacity() << endl;

	s0 = "abc";
	cout << s0.size() << endl;
	cout << s0.capacity() << endl;

	s0 += "abcdefghijklmnoprrakdfl;";
	cout << s0.size() << endl;
	cout << s0.capacity() << endl;

}

void main6()
{
	string s0;
	ifstream ifs("randstr.txt");
	size_t index;

	if (!ifs.is_open())
	{
		cout << "failed to open file" << endl;
		exit(EXIT_FAILURE);
	}
	getline(ifs, s0);
	cout << s0.size() << endl;

	string s1;
	cout << "what string do you want:";
	cin >> s1;
	while (!cin.fail() && !cin.eof())
	{
		index = s0.find(s1);
		if (index == string::npos)
			cout << "cannot find " << s1 << endl;
		else
			cout << s1 << " is at " << index << endl;
		cout << "what string do you want:";
		cin >> s1;
	}
}

void main5()
{
	ofstream ofs("randstr.txt");

	const char *alpha = "abcdefghijklmnopqrstuvwxyz";
	for (int i = 0; i < 1024 * 1024; i++)
		ofs << alpha[rand() % 26];
	ofs.close();
}

void main4()
{
	bool quit = false;
	string s0;
	ifstream ifs;
	
	ifs.open("of.txt");
	if (ifs.is_open())
		cout << "open success" << endl;
	else {
		cout << "open failed" << endl;
		exit(EXIT_FAILURE);
	}

	//getline(ifs, s0, ':'); // use ':' use seperator
	getline(ifs, s0);
	while (1)
	{
		if (ifs.fail())
		{
			cout << "ifs.fail()" << endl;
			quit = true;
		}
		if (ifs.eof())
		{
			cout << "ifs.eof()" << endl;
			quit = true;
		}
		if (quit)
			break;
		cout << s0 << endl;
		getline(ifs, s0);
	}
	ifs.close();
}

void main3()
{
	ofstream of("of.txt");

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
			of << rand() << " ";
		of << endl;
	}

	of.close();
}

void main2()
{
	const char *hello = "hello";
	const char *alpha = "abcdefghijklmnopqrstuvwxyz";

	string s0(10, 'a');
	cout << s0 << endl;

	string s1(hello, 5); // init with the first 5 chars of hello
	cout << s1 << ", " << s1.size() << endl;

	string s2;
	s2 += hello;
	cout << s2 << ", " << s2.size() << endl;

	string s3(alpha + 5, alpha + 10);
	cout << s3 << ", " << s3.size() << endl;

	string s4(alpha);
	string s5(&s4[10], &s4[20]);
	cout << s5 << ", " << s5.size() << endl;

	string s6(s4, 0, 23);
	cout << s6 << ", " << s6.size() << endl;

	string s7 = { '1', '2', '3', '4', '5' };
	cout << s7 << ", " << s7.size() << endl;
}

void main1()
{
	string s0;

	while (!cin.fail() && !cin.eof())
	{
		//getline(cin, s0);
		cin >> s0;
		char *p = new char [s0.size() + 1];
		int i = 0;
		/*
		for (i = 0; i < s0.size(); i++)
			p[i] = s0[i];
		*/
		for (string::iterator it = s0.begin(); it != s0.end(); it++)
		{
			cout << *it << ",";
			p[i++] = *it;
		}
		p[i] = '\0';
		int n = atoi(p);
		cout << n << endl;
		//cout << s0 << endl;
		delete [] p;

		if (n == 1245)
		{
			cout << "quit for magic number" << endl;
			break;
		}
	}
}

int main()
{
	main14();

	return 0;
}
