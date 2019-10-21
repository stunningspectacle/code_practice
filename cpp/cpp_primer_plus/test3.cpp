#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <bitset>
using namespace std;

class A {
public:
	int pub0;
	void show() {
		map<void *, string> m;
		m.emplace(&pub0, "pub0");
		m.emplace(&pub1, "pub1");
		m.emplace(&priv1, "priv1");
		m.emplace(&priv0, "priv0");
		m.emplace(&prot0, "prot0");
		m.emplace(&prot1, "prot1");

		for (auto &p: m)
			cout << p.second << "@" << p.first << endl;
	}

protected:
	int prot0;
private:
	int priv0;
protected:
	int prot1;
private:
	int priv1;
public:
	int pub1;
};

int main()
{
	bitset<64> b;

	for (int i = 0; i < 10; i++) {
		int t = rand() % 64;
		b[t] = 1;
		cout << t << " ";
	}
	cout << endl;
	cout << b.size() << endl;

	for (int i = 0; i < 64; i++)
		if (b[i])
		cout << i << " ";
	cout << endl;

	return 0;
}
