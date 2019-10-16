#include <iostream>
#include <string>
#include <valarray>

using std::cout;
using std::endl;
using std::string;
using std::valarray;
using std::ostream;

template <typename TT> void show(const TT &);

template <typename T>
class Stack
{
	//T *items;
	valarray<T> items;
	int top;
	int size;
	int max_size;
public:
	Stack(int ms = 128);
	~Stack();
	bool push(T t);
	bool pop(T &t);
	bool isempty() const { return size == 0; };
	bool isfull() const { return (size == max_size); };
	friend ostream & operator<<(ostream & os, const Stack<T> &s);
	friend void show<>(const Stack<T> &); // bounded friend(bounded to class)
	//friend void show<Stack<T>>(const Stack<T> &); // Also OK
	template <typename A> friend void show2(const A&); //Non bounded friend
};

template class Stack<double>; // explicit instantiation

template <> class Stack<string> // explicit specification
{
	string s;
public:
	Stack() { s = "aaaa"; };
	void show() const { cout << s << endl; };
};

template <typename T>
Stack<T>::Stack(int ms)
{
	//items = new T [ms];
	//items = new valarray<int>(ms);
	items = valarray<T>(ms);
	max_size = ms;
	size = top = 0;
}

template <typename T>
Stack<T>::~Stack()
{
	//delete [] items;
	//delete items;
}

template <typename T>
bool Stack<T>::push(T t)
{
	cout << "pushed " << t << endl;
	if (isfull())
		return false;
	cout << "top=" << top << endl;
	items[top++] = t;
	size++; 

	return true;
}

template <typename T>
bool Stack<T>::pop(T &t)
{
	if (isempty())
		return false;
	t = items[--top];
	size--;

	return true;
}

ostream & operator<<(ostream & os, const Stack<int> &s)
{
	if (s.isempty())
		os << "stack is empty" << endl;
	else 
		for (int i = s.top - 1; i >= 0; i--)
			os << s.items[i] << "-->";

	return os;
}

ostream & operator<<(ostream & os, const Stack<double> &s)
{
	if (s.isempty())
		os << "stack is empty" << endl;
	else 
		for (int i = s.top - 1; i >= 0; i--)
			os << s.items[i] << "-->";

	return os;
}

template <typename T>
void show(const T &s)
{
	cout << s.top << " " << s.size << endl;
}

template <typename A>
void show2(const A &s)
{
	cout << s.top << " " << s.size << endl;
}

int main()
{
	Stack<int> s0;
	

	cout << s0 << endl;

	s0.push(100);
	cout << s0 << endl;

	s0.push(200);
	cout << s0 << endl;

	/*
	s0.pop(t);
	cout << s0 << endl;
	s0.pop(t);
	cout << s0 << endl;
	*/

	valarray<int> v0;
	v0 = valarray<int>(100);

	Stack<double> s1;
	s1.push(123.22);
	cout << s1 << endl;

	Stack<string> s2;
	s2.show();

	show(s0);
	show(s1);

	show2(s0);
	show2(s1);

	return 0;
}
