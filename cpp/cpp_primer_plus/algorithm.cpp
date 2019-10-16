#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <iterator>
#include <utility>
#include <tuple>
#include <random>
#include <chrono>

using namespace std;

static const int N = 20;

template <typename T>
void show(T &t)
{
	for (auto &v: t)
		cout << v << " ";
	cout << endl;
}

void test_all_of(vector<int> &v)
{
	if (all_of(v.begin(), v.end(), [](int i) { return i > 0; }
				))
		cout << __func__ << ": yes" << endl;
	else
		cout << __func__ << ": no" << endl;
}

void test_none_of(vector<int> &v)
{
	if (none_of(v.begin(), v.end(), [](int i) { return i > 100; }))
		cout << __func__ << ": yes" << endl;
	else
		cout << __func__ << ": no" << endl;
}

void test_any_of(vector<int> &v)
{
	if (any_of(v.begin(), v.end(), [](int i) { return i > 50; }))
		cout << __func__ << ": yes" << endl;
	else
		cout << __func__ << ": no" << endl;
}

int non_modifying_sequence_ops()
{
	vector<int> v0 = { 20, 10, 200, 200, 30, 9, 20, 10, 200, 40, 7, 8};
	vector<int> v2 = { 20, 10, 200, 200, 30, 9, 20, 10, 200, 40, 7 };
	vector<int> v3 = { 8, 10, 200, 200, 30, 9, 20, 10, 200, 40, 7, 20};

	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl;

	vector<int>::iterator it;

	it = find(v0.begin(), v0.end(), 83);
	cout << "find(): " << *it << endl;

	it = find_if(v0.begin(), v0.end(), [](int i) { return i == 100; });
	cout << "find_if(): " << *it << endl;

	it = find_if_not(v0.begin(), v0.end(), [](int i) { return i == 83; });
	cout << "find_if_not():" << *it << endl;

	vector<int> v1 = { 10, 200, 40};
	it = find_end(v0.begin(), v0.end(), v1.begin(), v1.end());
	cout << "find_end(): " << it - v0.begin() << endl;

	it = find_first_of(v0.begin(), v0.end(), v1.begin(), v1.end());
	cout << "find_first_of(): " << it - v0.begin() << endl;

	it = adjacent_find(v0.begin(), v0.end());
	cout << "find_end(): " << it - v0.begin() << endl;

	int nr;
	nr = count(v0.begin(), v0.end(), 200);
	cout << "count(): " << nr << endl;

	nr = count_if(v0.begin(), v0.end(),
			[](int i) {
				return i > 10 && i < 200;
				});
	cout << "count_if(): " << nr << endl;

	if (v0 == v2)
		cout << "v0 == v2" << endl;
	else
		cout << "v0 != v2" << endl; 
	vector<int>::iterator it0, it1;
	pair<vector<int>::iterator, vector<int>::iterator> mypair;
	mypair = mismatch(v0.begin(), v0.end(), v2.begin());
	cout << "mismatch(): " << *mypair.first << " " << *mypair.second << endl;
	tie(it0, it1) = mypair;
	cout << "tuple: " << *it0 << " " << *it1 << endl;


	bool equ = equal(v0.begin(), v0.end(), v2.begin());
	cout << "equal(): " << equ << endl;

	bool perm = is_permutation(v0.begin(), v0.end(), v3.begin());
	cout << "is_permutation(): " << perm << endl;

	it = search(v0.begin(), v0.end(), v1.begin(), v1.end());
	cout << "search(): " << distance(v0.begin(), it) << endl;

	it = search_n(v0.begin(), v0.end(), 2, 200);
	cout << "search_n(): " << distance(v0.begin(), it) << endl;

}

void modifying_sequence_ops()
{
	vector<int> v0 = { 2, 1, 3, 9, 8, 6, 7 };
	vector<int> v1 = { 22, 11, 33, 99, 88, 66, 77 };
	vector<int> v2 = { 222, 111, 333, 999, 888, 666, 777 };
	vector<int> v3 = { 2222, 1111, 3333, 9999, 8888, 6666, 777 };

	cout << "v0: ";
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl;

	insert_iterator< vector<int> > insert(v1, v1.end());
	copy(v0.begin(), v0.end(), insert);
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));
	for (auto &v: v0)
		v1.push_back(v);
	cout << endl << "copy(): ";
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "copy_n(): ";
	copy_n(v3.begin(), 3, v1.begin());
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "copy_if(): ";
	copy_if(v3.begin(), v3.end(), v1.begin(), [](int n) { return (n % 2 == 0);});
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "copy_backward(): ";
	copy_backward(v3.begin(), v3.end(), v1.end());
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "move(): ";
	move(v1.begin(), v1.begin() + 5, v2.begin());
	copy(v2.begin(), v2.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "move_backward(): ";
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl;
	copy(v2.begin(), v2.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl;
	move_backward(v1.begin(), v1.begin() + 5, v2.end());
	copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl;
	copy(v2.begin(), v2.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "swap(): ";
	swap(v0[0], v0[1]);
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "iter_swap(): ";
	iter_swap(v0.begin(), --v0.end());
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "transform(): ";
	transform(v0.begin(), v0.end(), v0.begin(), [](int n) { return 2 * n; });
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl;
	for_each(v0.begin(), v0.end(), [](int n) { return 2 * n; });
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "replace():";
	replace(v0.begin(), v0.end(), 12, 99);
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "replace_if():";
	replace_if(v0.begin(), v0.end(), [](int n) { return n > 10;}, 3);
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "replace_copy(): ";
	replace_copy(v0.begin(), v0.end(),
			ostream_iterator<int, char>(cout, " "),
			3, 8);

	cout << endl << "replace_copy_if(): ";
	replace_copy_if(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "),
				[](int n) { return n < 10; }, 9);

	cout << endl << "fill(): ";
	fill(v0.begin(), v0.end(), 1);
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "fill_n(): ";
	fill_n(v0.begin(), 3, 5);
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "generate(): ";
	srand(time(0));
	generate(v0.begin(), v0.end(), []() { return rand() % 10; });
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	cout << endl << "generate_n(): ";
	generate_n(v0.begin(), 3, []() { return rand() % 100; });
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	vector<int> v4 = { 1, 2, 3, 3, 3, 3, 4, 4, 4 };
	cout << endl << "v4.size(): " << v4.size();;
	cout << endl << "remove(): ";
	remove(v4.begin(), v4.end(), 3);
	copy(v4.begin(), v4.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl << "v4.size(): " << v4.size();

	cout << endl << "remove_copy(): ";
	remove_copy(v4.begin(), v4.end(), v0.begin(), 3);
	copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

	vector<int> v5 = { 1, 2, 3, 4, 4, 4, 4, 4 };
	cout << endl << "unique(): ";
	cout << "v5.size(): " << v5.size() << endl;;
	vector<int>::iterator it = unique(v5.begin(), v5.end());
	cout << distance(v5.begin(), it) << endl;;
	cout << "v5.size(): " << v5.size() << endl;;
	copy(v5.begin(), it, ostream_iterator<int, char>(cout, " "));

	vector<int> v6;
	cout << endl << "unique_copy(): ";
	//insert_iterator< vector<int> > insert_it(v6, v6.begin());
	unique_copy(v5.begin(), v5.end(), insert_iterator< vector<int> >(v6, v6.begin()));
	copy(v6.begin(), v6.end(), ostream_iterator<int, char>(cout, " "));


	cout << endl << "reverse(): ";
	reverse(v6.begin(), v6.end());
	copy(v6.begin(), v6.end(), ostream_iterator<int, char>(cout, " "));

	vector<int> v7;
	cout << endl << "reverse_copy(): ";
	reverse_copy(v6.begin(), v6.end(),
			insert_iterator< vector<int> >(v7, v7.begin()));
	copy(v7.begin(), v7.end(), ostream_iterator<int, char>(cout, " "));
	v7.push_back(5); v7.push_back(6); v7.push_back(7);

	cout << endl << "rotate(): ";
	rotate(v7.begin(), v7.begin() + v7.size() / 2, v7.end());
	copy(v7.begin(), v7.end(), ostream_iterator<int, char>(cout, " "));
	rotate(v7.begin(), v7.begin() + v7.size() / 2, v7.end());

	cout << endl << "rotate_copy(): ";
	vector<int> v8;
	rotate_copy(v7.begin(), v7.begin() + v7.size() / 2, v7.end(),
			insert_iterator< vector<int> >(v8, v8.begin()));
	copy(v7.begin(), v7.end(), ostream_iterator<int, char>(cout, " "));
	cout << endl;
	copy(v8.begin(), v8.end(), ostream_iterator<int, char>(cout, " "));

	vector<int> v9;
	for (int i = 0; i < 10; i++)
		v9.push_back(i);
	cout << endl << "random_shuffle(): ";
	random_shuffle(v9.begin(), v9.end());
	copy(v9.begin(), v9.end(), ostream_iterator<int, char>(cout, " "));

	sort(v9.begin(), v9.end());
	cout << endl << "shuffle(): ";
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	shuffle(v9.begin(), v9.end(), default_random_engine(seed));
	copy(v9.begin(), v9.end(), ostream_iterator<int, char>(cout, " "));
}

void test_partition()
{
	list<int> l0;
	vector<int> v0;
	for (int i = 0; i < 10; i++) {
		l0.push_back(i);
		v0.push_back(i);
	}

	cout << "l0: ";
	copy(l0.begin(), l0.end(), ostream_iterator<int, char>(cout, " "));

	{
		bool is_part;
		is_part = is_partitioned(l0.begin(), l0.end(), [](int i) { return i % 2 == 0; });
		cout << "is_partitioned(): " << is_part << endl;;

		cout << "partition(): ";
		partition(l0.begin(), l0.end(), [](int i) { return i % 2 == 0; });
		is_part = is_partitioned(l0.begin(), l0.end(), [](int i) { return i % 2 == 0; });
		copy(l0.begin(), l0.end(), ostream_iterator<int, char>(cout, " "));
		cout << "is_partitioned(): " << is_part << endl;;

		stable_partition(v0.begin(), v0.end(), [](int i) { return i % 2 == 0; });
		copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

		cout << endl << "partition_point(): ";
		cout <<
			partition_point(v0.begin(), v0.end(), [](int i) { return i % 2 == 0;})
			- v0.begin() << endl;
	}
}

void test_sort()
{
	{
		vector<string> v0 = { "aaa", "ccc", "bbb", "ddd" };
		copy(v0.begin(), v0.end(), ostream_iterator<string, char>(cout, ";"));

		cout << endl << "sort(): ";
		sort(v0.begin(), v0.end());
		copy(v0.begin(), v0.end(), ostream_iterator<string, char>(cout, ";"));

		cout << endl << "stable_sort(): ";
		stable_sort(v0.begin(), v0.end());
		copy(v0.begin(), v0.end(), ostream_iterator<string, char>(cout, ";"));
		cout << endl;
	}

	{
		vector<int> v0;
		for (int i = 0; i < 20; i++)
			v0.push_back(i);
		random_shuffle(v0.begin(), v0.end());
		copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

		cout << endl << "partial_sort(): ";
		partial_sort(v0.begin(), v0.begin() + v0.size()/2, v0.end());
		copy(v0.begin(), v0.end(), ostream_iterator<int, char>(cout, " "));

		vector<int> v1(20);
		cout << endl << "partial_sort_copy(): ";
		partial_sort_copy(v0.begin(), v0.end(), v1.begin(), v1.end());
		copy(v1.begin(), v1.end(), ostream_iterator<int, char>(cout, " "));

		cout << endl << "is_sorted(): " << is_sorted(v1.begin(), v1.end()) << endl;

		cout << "is_sorted_until(): " <<
			distance(v1.begin(), is_sorted_until(v1.begin(), v1.end())) << endl;

		cout << "before nth_element():" << endl;
		random_shuffle(v1.begin(), v1.end());
		show(v1);
		cout << "after nth_element():" << endl;
		nth_element(v1.begin(), v1.begin() + v1.size() / 2, v1.end());
		show(v1);

		sort(v1.begin(), v1.end());
		show(v1);
		cout << "lower_bound()of 12: " << *lower_bound(v1.begin(), v1.end(), 12) << endl;
		cout << "upper_bound()of 12: " << *upper_bound(v1.begin(), v1.end(), 12) << endl;

		v1[3] = 10; v1[4] = 10; v1[7] = 10;
		sort(v1.begin(), v1.end());
		show(v1);
		pair<vector<int>::iterator, vector<int>::iterator> p =
				equal_range(v1.begin(), v1.end(), 10);
		vector<int>::iterator a, b;
		tie(a, b) = p;
		cout << p.first - v1.begin() << " " << p.second - v1.begin() << endl;
		cout << a - v1.begin() << " " << b - v1.begin() << endl;

		cout << "binary_search(): " << binary_search(v1.begin(), v1.end(), 19) << endl;

		sort(v0.begin(), v0.end());
		sort(v1.begin(), v1.end());
		cout << "v0: ";
		show(v0);
		cout << "v1: ";
		show(v1); 
		vector<int> v2;
		merge(v0.begin(), v0.end(), v1.begin(), v1.end(),
				insert_iterator< vector<int> >(v2, v2.begin()));
		cout << "merge to v2: ";
		show(v2);

		random_shuffle(v2.begin(), v2.end());
		show(v2);
		sort(v2.begin(), v2.begin() + v2.size() / 2);
		sort(v2.begin() + v2.size() / 2, v2.end());
		show(v2);
		cout << "inplace_merge():";
		inplace_merge(v2.begin(), v2.begin() + v2.size() / 2, v2.end());
		show(v2);

		auto arr = {6, 6, 7, 8, 8};
		cout << "includes: " <<
			includes(v2.begin(), v2.end(), arr.begin(), arr.end()) << endl;
	}

	{
		vector<int> v0 = {0, 0, 1, 1, 1, 2, 2};
		vector<int> v1 = {0, 0, 0, 1, 1, 2, 2, 3};
		vector<int> v2, v3, v4, v5;

		cout << "set_union():" << endl;
		set_union(v0.begin(), v0.end(), v1.begin(), v1.end(),
				insert_iterator< vector<int> >(v2, v2.begin()));
		show(v2);

		cout << "set_intersection():" << endl;
		set_intersection(v0.begin(), v0.end(), v1.begin(), v1.end(),
				insert_iterator< vector<int> >(v3, v3.begin()));
		show(v3);

		cout << "set_difference():" << endl;
		set_difference(v0.begin(), v0.end(), v1.begin(), v1.end(),
				insert_iterator< vector<int> >(v4, v4.begin()));
		show(v4);

		cout << "set_symmetric_difference():" << endl;
		set_symmetric_difference(v0.begin(), v0.end(), v1.begin(), v1.end(),
				insert_iterator< vector<int> >(v5, v5.begin()));
		show(v5);
	}
}

void test_heap()
{
	{
		vector<int> v0;
		for (int i = 0; i < 20; i++)
			v0.push_back(i);
		show(v0);
		cout << "is_heap:" << is_heap(v0.begin(), v0.end()) << endl;

		make_heap(v0.begin(), v0.end());
		show(v0);

		cout << "is_heap:" << is_heap(v0.begin(), v0.end()) << endl;
		cout << "front():" << v0.front() << endl;
		v0.push_back(90);
		cout << "after push_back(90)" << endl;
		show(v0);

		cout << "after push_heap()" << endl;
		push_heap(v0.begin(), v0.end());
		show(v0);

		cout << "after pop_heap()" << endl;
		pop_heap(v0.begin(), v0.end());
		show(v0);

		cout << "after pop_back()" << endl;
		v0.pop_back();
		show(v0);

		cout << "sort_heap()" << endl;
		sort_heap(v0.begin(), v0.end());
		show(v0);

		cout << "make_heap() size()/2" << endl;
		make_heap(v0.begin(), v0.begin() + v0.size()/2);
		show(v0);

		cout << "is_heap_until():" << endl;
		cout << is_heap_until(v0.begin(), v0.end()) - v0.begin() << endl;
	}
}

bool mycomp(int a, int b)
{
	return a<b;
}

void min_max()
{
	{
		auto p = minmax(100, 20);
		cout << p.first << " " << p.second << endl;

		pair<int, int> p0 = minmax(123, 22);
		cout << p0.first << " " << p0.second << endl;

		int a, b;
		tie(a, b) = p0;
		cout << a << " " << b << endl;

		vector<int> v0;
		for (int i = 0; i < 20; i++)
			v0.push_back(rand() % 100);
		show(v0);

		cout << "min_element(): " << *min_element(v0.begin(), v0.end()) << endl;
		cout << "max_element(): " << *max_element(v0.begin(), v0.end()) << endl;

		auto p1 = minmax_element(v0.begin(), v0.end());
		cout << *p1.first << " " << *p1.second << endl;

		int v = *min_element(v0.begin(), v0.end(), 
				[](int a, int b) {
					a = a > 0 ? a : -a;
					b = b > 0 ? b : -b;

					return a<b;
				});
		cout << v << endl;

	}

	{
		list<int> l0;
		for (int i = 0; i < 20; i++)
			l0.push_back(i);
		show(l0);

		//auto p = l0.end();
		l0.erase(remove(l0.begin(), l0.end(), 3), l0.end());
		l0.erase(remove(l0.begin(), l0.end(), 4), l0.end());
		l0.erase(remove(l0.begin(), l0.end(), 5), l0.end());
		for (auto p0 = l0.begin(); p0 != l0.end(); p0++)
			cout << *p0 << " ";
		cout << endl;
	}

	{
		vector<int> v0 = { 1, 2, 3, 4, 5, 6 };
		show(v0);
		next_permutation(v0.begin(), v0.end());
		show(v0);

		sort(v0.begin(), v0.end(), [](int a, int b) { return a > b; });
		show(v0);
		next_permutation(v0.begin(), v0.end());
		show(v0);
	}
}

int main()
{
	min_max();
}
