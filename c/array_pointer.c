#include <stdio.h>

struct test {
	int a;
	int b;
	int c[];
};

struct test test0;

int main(void)
{
	printf("%p, %p\n", test0.c, &test0.c);

	return 0;
}
