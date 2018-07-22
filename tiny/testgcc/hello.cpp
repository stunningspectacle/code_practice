#include <stdio.h>

class BeforeHello {
public:
	BeforeHello();
	~BeforeHello();
};

BeforeHello::BeforeHello()
{
	printf("Say it before main\n");
}

BeforeHello::~BeforeHello()
{
	printf("Say it after main\n");
}

BeforeHello before;

int main()
{
	printf("Hello World!\n");
	return 0;
}
