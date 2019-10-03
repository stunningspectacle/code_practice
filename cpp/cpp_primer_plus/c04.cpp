#include <iostream>
#include <cstring>
#include <string>
using namespace std;

void main9()
{
	int nr_names;
	char temp[80];

	cout << "How many names do you want?" << endl;
	cin >> nr_names;

	cout << "need " << nr_names << " names" << endl;
	char **names = new char * [nr_names];

	cin.get();
	for (int i = 0; i < nr_names; i++) {
		//cin.get(temp, 80).get();
		cin.getline(temp, 80);
		names[i] = new char [strlen(temp) + 1];
		strcpy(names[i], temp);
	}

	for (int i = 0; i < nr_names; i++) {
		cout << names[i] << endl;
		delete names[i];
	}

	delete [] names;
}

void main8()
{
	int num = 0;
	string *ss;

	cout << "how many strings do you want?" << endl;
	cin >> num;
	cout << "will create " << num << " strings" << endl;

	ss = new string[num];
	for (int i = 0; i < num; i++) {
		ss[i] = "A";
		ss[i][0] += i;
	}
	for (int i = 0; i < num; i++)
		cout << i << ":" << ss[i] << ", ";
	cout << endl;


	cout << ss << ", " << ss + 1 << ", "
		<< (unsigned long)(ss + 1) - (unsigned long)ss
		<< endl;

	cout << ss << endl;
	delete [] ss;
	cout << ss << endl;

	for (int i = 0; i < num; i++)
		cout << i << ":" << ss[i] << ", ";
	cout << endl;
}

void main7()
{
	struct inflatable {
		string name;
		int value;
	};


	inflatable x = {
		"aaa",
		100,
	};
	inflatable y = {
		"bbb",
		200,
	};

	inflatable z;
	z.name = x.name;

	cout << x.name << ", " << x.value << endl;
	cout << y.name << ", " << y.value << endl;
	cout << z.name << ", " << z.value << endl;

	struct student {
		char name[20];
		int value;
	};

	student s0 = { "AAA", 100 };
	cout << s0.name << ", " << s0.value << endl;

	student s1;

	s1 = s0;
	cout << s1.name << ", " << s1.value << endl;
}

void main6()
{
	string s0 = "aabb";

	cout << s0 << endl;
	s0 += "ccdd";
	cout << s0 << endl;

	cout << s0.size() << endl;

	cin >> s0;
	cout << s0 << endl;
}

void main5()
{
	const int BUF_SIZE = 4;

	char name[BUF_SIZE] = { };
	char str[BUF_SIZE] = { };

	cout << "Name:";
	cin.get(name, BUF_SIZE).get();

	cout << "str:";
	cin.get(str, BUF_SIZE).get();

	cout << name << ", " << str << endl;
}

void main4()
{
	char name[32] = { };
	char dessert[128] = { };

	cout << "Input your name:";
	//cin >> name;
	cin.getline(name, sizeof(name));

	cout << "Input your favorite dessert:";
	//cin >> dessert;
	cin.getline(dessert, sizeof(dessert));

	cout << "I have some " << dessert << " for you " << name << endl;
}

void main3()
{
	char name[5];

	for (int i = 0; i < sizeof(name); i++)
		name[i] = 'a';

	for (int i = 0; i < sizeof(name); i++)
		cout << int(name[i]) << " ";
	cout << endl;

	cout << name << endl;
	cout << "input name:";
	cin >> name;
	for (int i = 0; i < sizeof(name); i++)
		cout << int(name[i]) << " ";

	cout << endl << "strlen(name):" << strlen(name) << endl;
}

void main2()
{
	char hello[] = "hello";

	cout << hello << endl;

	hello[0] = 'K';
	cout << hello << endl;
}

void main1()
{
	int num[10] = { }; // Will be init to all 0

	for (int i = 0; i < sizeof(num) / sizeof(num[0]); i++)
		cout << num[i] << " ";
	cout << endl;
}

int main()
{
	main9();

	return 0;
}
