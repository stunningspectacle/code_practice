#include <iostream>
#include <valarray>

using std::cout;
using std::endl;
using std::valarray;

int main()
{
	valarray<int> nums = {1, 2, 3, 4};
	valarray<double> scores = { 0.23, 0.45, 0.33, 0.56 };

	for (int i = 0; i < nums.size(); i++)
		cout << nums[i] << endl;

	for (int i = 0; i < scores.size(); i++)
		cout << scores[i] << endl;

	valarray<int> v0(nums); 
	valarray<int> v1(10, 8);

	cout << "v0:" << endl;
	for (int i = 0; i < v0.size(); i++)
		cout << v0[i] << endl;
	cout << "sum:" << v0.sum() << endl;

	cout << "v1:" << endl;
	for (int i = 0; i < v1.size(); i++)
		cout << v1[i] << endl;
	cout << "sum:" << v1.sum() << endl;

	return 0;
}
