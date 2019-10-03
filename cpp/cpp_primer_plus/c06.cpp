#include <iostream>
#include <fstream>
#include <cctype>
using namespace std;

int jack(int lines)
{
	return lines * 2;
}

int john(int lines)
{
	return lines * 5;
}

int estimate(int lines, int (*fn)(int))
{
	return (*fn)(lines);
}

void main3()
{
	int lines;

	cout << "How many lines of code do you need?" << endl;
	cin >> lines;

	cout << "Jack needs " << estimate(lines, jack) << " lines" << endl;

	cout << "John needs " << estimate(lines, john) << " lines" << endl;
}

void main2()
{
	ofstream ofs;
	ifstream ifs;

	ofs.open("a.txt");
	if (ofs.good())
		cout << "it's good" << endl;
	ofs << "This is a file" << endl;
	ofs.close();

	ifs.open("a.txt");
	if (ifs.good())
		cout << "it's good" << endl;
	if (ifs.fail())
		cout << "it's fail" << endl;

	char s[80];
	int fail = false;
	while (1) {
		ifs.getline(s, 80);
		if (ifs.eof()) {
			cout << "it's eof" << endl;
			fail = true;
		}
		if (ifs.fail()) {
			cout << "it's fail" << endl;
			fail = true;
		}
		if (fail)
			break;
		cout << s << endl;
	}
	ifs.close();

	if (ifs.is_open())
		cout << "yes, it's open" << endl;
	else
		cout << "no, it's closed" << endl;
}

void main1()
{
	char ch;

	cin.get(ch);
	while (cin) {
		if (ch != '\n') {
			if (isalpha(ch))
				cout << ch << " is alpha" << endl;
			if (isdigit(ch))
				cout << ch << " is digit" << endl;
			if (ispunct(ch))
				cout << ch << " is punct" << endl;
			if (isspace(ch))
				cout << ch << " is space" << endl;
			if (isalnum(ch))
				cout << ch << " is alnum" << endl;
			if (isblank(ch))
				cout << ch << " is blank" << endl;
			if (isprint(ch))
				cout << ch << " is print" << endl;
			if (islower(ch))
				cout << ch << " is lower" << endl;
			if (isupper(ch))
				cout << ch << " is upper" << endl;
		}
		cin.get(ch);
	}
}

int main()
{
	main3();

	return 0;
}
