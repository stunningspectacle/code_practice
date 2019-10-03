#include <iostream>
#include <cmath>
#include <cstdlib>


void main7()
{
	int i, cnt = 0;

	std::cout << "how many random numbers do you want?" << std::endl;
	std::cin >> cnt;
	for (i = 0; i < cnt; i++)
		std::cout << rand() % 100 << " ";
	std::cout << std::endl;
}

void main6()
{

	float a = sqrt(6.22);

	std::cout << "main6: a=" << a << std::endl;
}

void main5()
{
	using namespace std;

	int carrots = 0;

	cout << "How many carrots do you have?" << endl;
	//cin >> carrots;
	cout << "OH, you have " << carrots << endl;
}

void main4()
{
	using namespace std;

	int num = 100;

	cout << "main4" << endl;
	cout << "Now I have " << num << " goods" << endl;

	num -= 1;

	cout << "Now I have " << num << " goods" << endl;
}

void main3()
{
	using std::cout;
	using std::endl;

	cout << "this is main3" << endl;
}

void main2()
{
	std::cout << "this is main2" << std::endl;
}

int main()
{
	using namespace std;
	cout << "Come up and c++ me sometime";
	cout << endl;
	cout << "You won't regret it!" << endl;

	main2();
	main3();
	main4();
	main5();
	main6();
	main7();

	return 0;
}
