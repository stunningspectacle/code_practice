#include <iostream>
using std::cout;
using std::endl;

template <typename Type>
class Stack {
    Type array[10];
    int num;
public:
    Stack(int a = 10);
    bool isFull();
};


template <typename Type>
bool Stack<Type>::isFull() {
    return num == 10;
}

template <typename Type>
Stack<Type>::Stack(int a) {
    cout << "Yes, create" << a << endl;
}

int main()
{
    Stack<int> mystack = Stack<int>();
    cout << mystack.isFull() << endl;

    return 0;
}
