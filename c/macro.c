#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct path {
	int a;
	int b;
};

struct test {
	int a;
	struct path path;
#define pa path.a
#define pb path.b
	int c;
	int d;
};

struct test test1;

void main(void)
{
	struct test test0;

	struct test *p = &test1;

	printf("%d\n", test0.pa);
	printf("%d\n", p->pa);
}

