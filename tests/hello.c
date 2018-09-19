#include <stdio.h>

struct {
	int a;
	int b;
	int c;
} test;

void main() 
{
	test.a = 10;
	test.b = 20;
	test.c = 30;

	printf("%d, %d, %d\n", test.a, test.b, test.c);
}
