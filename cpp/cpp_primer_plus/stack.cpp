#include <iostream>
#include <string>
#include <valarray>

using std::cout;
using std::endl;
using std::string;
using std::valarray;
using std::ostream;

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

int main()
{
	Stack<int> s0;
	

	cout << s0 << endl;

	s0.push(100);
	cout << s0 << endl;

	s0.push(200);
	cout << s0 << endl;

	s0.pop(t);
	cout << s0 << endl;
	s0.pop(t);
	cout << s0 << endl;

	valarray<int> v0;
	v0 = valarray<int>(100);

	return 0;
}
