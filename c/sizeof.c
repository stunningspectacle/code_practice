#include <stdlib.h>
#include <stdio.h>

struct A {
	char a;
};
struct B {
	char b0;
	char b1;
};
struct C {
	char c0;
	int c1;
	char c2;
};
struct D {
	char d0;
	unsigned long d1;
	char d2;
};

int main(void)
{
	float f;
	int a = 10;
	int b = 3;
	printf("sizeof(double)=%d\n", sizeof(double));
	printf("sizeof(long double)=%d\n", sizeof(long double));
	printf("sizeof(long)=%d\n", sizeof(long));
	printf("sizeof(long long)=%d\n", sizeof(long long));
	printf("sizeof(short)=%d\n", sizeof(short));
	printf("sizeof(short int)=%d\n", sizeof(short int));
	printf("sizeof A(%d), B(%d), C(%d), D(%d)\n",
			sizeof(struct A), sizeof(struct B), sizeof(struct C), sizeof(struct D));
	f = (float)a / b;
	printf("%f\n", f);
}
