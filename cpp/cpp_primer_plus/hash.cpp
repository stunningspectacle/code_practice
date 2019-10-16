#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <unordered_map>

using namespace std;

template <typename T>
void show(T &t)
{
	for (auto &v : t)
		cout << v.first << "," << v.second << " ";
	cout << endl;
}

void test_hash()
{
	hash<int> hash_int;

	cout << hash_int(1) << endl;
	cout << hash_int(2) << endl;

	vector<int> v0 = { 23, 49, 59, 23, 88, 71, 8, 12 };
	map<int, int> m0;
	unordered_map<int, int> um0;

	for (int i = 0; i < v0.size(); i++) {
		m0.emplace(v0[i], i);
		um0.emplace(v0[i], i);
	}
	show(m0);
	show(um0);

	auto p = m0.find(59);
	cout << p->first << endl;
}

int main()
{
	test_hash();
}
