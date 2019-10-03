#include <string>
#include <iostream>
#include "shares.h"

using namespace std;

int main()
{
	Stock s0;
	s0.show();

	s0.acquire("Apple", 100, 125.3);
	s0.show();

	s0.sell(20);
	s0.show();

	Stock s1("Google", 50, 211.3);
	s1.show();

	s1.sell(20);
	s1.show();

	Stock s2;
	s2.show();

	s1.topval(s0);

	s2 = s0 + s1;
	s2.show();

	s2 = s2 * 10;
	s2.show();

	s2 = 10 *s2;
	s2.show();

	cout << s2;

	cout << s0 << s1 << s2;
	cout << -s0 << -s1 << -s2;

	int shares = s0;
	cout << shares << endl;

	return 0;
}
