#include <stdio.h>
#include <stdlib.h>

#define __weak __attribute__((weak))

struct A {
	int a;
	char b;
	int c;
	int d;
};

struct B {
	int a;
	char b;
	int c;
};

int var0 __weak;
int var1 = 80;
int var2 = 800;

int var0 __weak;

int main(void)
{
	struct A Aa = { 0 };
	struct B Ba = { 0 };


	struct A *Ap = &Aa;
	struct B *Bp;

	Bp = (struct B*)Ap;

	printf("var0=%d\n", var0);
}
