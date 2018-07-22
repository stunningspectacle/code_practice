#include <stdio.h>

#pragma pack(1)

struct test1 {
	char a;
	int b;
	char c;
	int d;
};

#pragma pack()
struct test2 {
	char a;
	int b;
	char c;
	int d;
};

void main(void)
{
	printf("sizeof(test1)=%d\n", sizeof(struct test1));
	printf("sizeof(test2)=%d\n", sizeof(struct test2));
}
