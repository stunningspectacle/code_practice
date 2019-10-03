#include <iostream>
#include <cstring>
#include <string>
#include <ctime>
#include <unistd.h>
using namespace std;

void main6()
{
	char ch;
	int cnt = 0;

	while (cin.get(ch))
		cnt++;
	cout << cnt << " characters" << endl;
}

void main5()
{
	int cnt = 0;
	char ch;

	cin.get(ch);
	while (cin.fail() == false) {
		cout << ch;
		cnt++;
		cin.get(ch);
	}

	cout << endl << cnt << " characters" << endl;
}

void main4()
{
	char ch;
	int cnt = 0;

	cout << "input something:" << endl;
	cin.get(ch);
	while (ch != '#') {
		cout << ch;
		cnt++;
		cin.get(ch);
	}

	cout << endl << cnt << " characters" << endl;
}

void main3()
{
	int cnt = 0;
	clock_t start;

	while (cnt < 10) {
		cout << cnt << "," << (start = clock())
			<< ", " << CLOCKS_PER_SEC << endl;
		sleep(1);
		cnt++;
	}

}

void main1()
{
	string word = "zate";
	string guess = "?ate";

	for (char ch = 'a'; guess != word; ch++) {
		cout << guess << endl;
		guess[0] = ch;
	}

	cout << "The word is '" << guess << "'"<< endl;
}

void main2()
{
	const char *word = "mate";
	char guess[] = "?ate";

	for (char ch = 'a'; strcmp(word, guess); ch++) {
		cout << guess << endl;
		guess[0] = ch;
	}
	cout << "The word is '" << guess << "'"<< endl;
}

int main()
{
	main6();

	return 0;
}
