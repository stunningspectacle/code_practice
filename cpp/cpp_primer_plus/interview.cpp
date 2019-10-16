#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

string longstr("zudfweormatjycujjirzjpyrmaxurectxrtqedmmgergwdvjmjtstdhcihacqnothgttgqfywcpgnuvwglvfiuxteopoyizgehkwuvvkqxbnufkcbodlhdmbqyghkojrgokpwdhtdrwmvdegwycecrgjvuexlguayzcammupgeskrvpthrmwqaqsdcgycdupykppiyhwzwcplivjnnvwhqkkxildtyjltklcokcrgqnnwzzeuqioyahqpuskkpbxhvzvqyhlegmoviogzwuiqahiouhnecjwysmtarjjdjqdrkljawzasriouuiqkcwwqsxifbndjmyprdozhwaoibpqrthpcjphgsfbeqrqqoqiqqdicvybzxhklehzzapbvcyleljawowluqgxxwlrymzojshlwkmzwpixgfjljkmwdtjeabgyrpbqyyykmoaqdambpkyyvukalbrzoyoufjqeftniddsfqnilxlplselqatdgjziphvrbokofvuerpsvqmzakbyzxtxvyanvjpfyvyiivqusfrsufjanmfibgrkwtiuoykiavpbqeyfsuteuxxjiyxvlvgmehycdvxdorpepmsinvmyzeqeiikajopqedyopirmhymozernxzaueljjrhcsofwyddkpnvcvzixdjknikyhzmstvbducjcoyoeoaqruuewclzqqqxzpgykrkygxnmlsrjudoaejxkipkgmcoqtxhelvsizgdwdyjwuumazxfstoaxeqqxoqezakdqjwpkrbldpcbbxexquqrznavcrprnydufsidakvrpuzgfisdxreldbqfizngtrilnbqboxwmwienlkmmiuifrvytukcqcpeqdwwucymgvyrektsnfijdcdoawbcwkkjkqwzffnuqituihjaklvthulmcjrhqcyzvekzqlxgddjoir");

template<typename T>
void show(T &t)
{
	for (auto &a : t)
		cout << a << " ";
	cout << endl;
}

int test0()
{

	std::vector<int> arr0;

	for (int i = 0; i < 10; i++)
		arr0.push_back(rand() % 100);

	std::for_each(arr0.begin(), arr0.end(),
			[](int n) { cout << n << " "; });

	std::for_each(arr0.begin(), arr0.end(),
			[](int n) { cout << n << " "; });

	auto p = std::find_if(arr0.begin(), arr0.end(), [](int n)
			{
				return n > 90;
			});
	cout << endl << "p = " << *p << endl;

	std::vector<int> arr1;
	std::unique(arr0.begin(), arr0.end());

	std::for_each(arr0.begin(), arr0.end(),
			[](int n) { cout << n << " "; });
	cout << endl;

	std::sort(arr0.begin(), arr0.end());
	std::for_each(arr0.begin(), arr0.end(),
			[](int n) { cout << n << " "; });
	cout << endl;

	std::random_shuffle(arr0.begin(), arr0.end());
	std::for_each(arr0.begin(), arr0.end(),
			[](int n) { cout << n << " "; });
	cout << endl;

	return 0;
}

vector<int> filter(vector<int> v0)
{
	sort(v0.begin(), v0.end());
	auto p = unique(v0.begin(), v0.end());
	v0.erase(p, v0.end());

	return v0;
}

int calc(string q, int x)
{
	int value = 0;
	int coef0 = 0, coef1 = 0, coef2 = 0;
	bool pos = true;
	const string s0("²");

	for (int i = 0; i < q.size(); i++) {
		if (q[i] == '-') {
			pos = false;
		} else if (q[i] == '+') {
			pos = true;
		} else if (q[i] == 'x') {
			if (value == 0)
				value = 1;
			if (i <= q.size() - 2 && q[i+1] == s0[0] && q[i+2] == s0[1]) {
				coef2 = pos ? value : -value;
				cerr << "coef2: " << coef2 << endl;
			} else {
				coef1 = pos ? value : -value;
				cerr << "coef1: " << coef1 << endl;
			}
			value = 0;
		} else if (isdigit(q[i]))
			value = value * 10 + q[i] - '0';
	}

	coef0 = pos ? value : -value;
	cerr << "coef0: " << coef0 << endl;

	cout << "result of " << q << ": " << coef2 * x * x + coef1 * x + coef0 << endl;
}

class A {
	int value;
public:
	A(int a): value(a) { }
	virtual void get_value() { cout << value << endl; }
};

class B: public A {
public:
	B(): A(1) { }
};

class C: public A {
public:
	C(): A(2) { }
};

class D: public B {
public:
	D(): B() { }
};

bool ispalin(string &s)
{
	string s0(s);
	reverse(s0.begin(), s0.end());
	return s0 == s;
}

class Solution {
public:
	static int lengthOfLongestSubstring(string s) {
		const char *str = s.data();
		int i = 0, j = 0;
		int n = s.size();
		int longest = 0;
		while (j < n) {
			auto p = find(str + i, str + j, str[j]);
			if (p == str + j) {
				longest = max(j - i + 1, longest);
				j++;
			} else {
				i = p - str + 1;
			}
		}
		return longest;
	}

	static double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
		show(nums1);
		show(nums2);
		vector<int> nums3;
		nums3.reserve(nums1.size() + nums2.size());
		merge(nums1.begin(), nums1.end(), nums2.begin(), nums2.end(),
				insert_iterator< vector<int> >(nums3, nums3.begin()));
		show(nums3);

		int size = nums3.size();
		cout << "size: " << size << endl;
		if (size % 2 == 0)
			return nums3[size/2];
		return (nums3[(size - 1) / 2 ] + nums3[(size + 1) / 2]) / 2;
	}

	static string longestPalindrome(string &s) {
		string ss;
		if (s.size() == 1)
			return s;

		for (int i = 0; i < s.size(); i++) {
			for (int j = i; j < s.size(); j++) {
				string s0(s.begin() + i, s.begin() + j + 1);
				if (ispalin(s0))
					if (s0.size() > ss.size())
						ss = s0;
			}
		}
		return ss;
	}

	/* longest comman string */
	static int lcs(string &s0, int len0, string &s1, int len1, int count)
	{
		if (len0 == 0 || len1 == 0)
			return count;
		if (s0[len0 - 1] == s1[len1 - 1])
			count = lcs(s0, len0 - 1, s1, len1 - 1, count + 1);
		count = max(count,
				max(lcs(s0, len0, s1, len1 - 1, 0),
					lcs(s0, len0 - 1, s1, len1, 0)));
		return count;
	}
};

int main()
{

}
