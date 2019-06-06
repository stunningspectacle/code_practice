#include "a.h"

#define min(a, b) ({ \
    const typeof(a) _a = (a); \
    const typeof(b) _b = (b); \
    (void) (&_a == &_b); \
    _a < _b ? _a : _b; })
#define pr_x(fmt, arg...) printf("xxxxx" fmt, ##arg)

struct num_t {
    int a;
    int b;
    int c;
    char abc;
};

struct minsize {
	char a;
	unsigned long i1;
	char b;
};

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

int func0(void)
{
	printf("ok");
}

int main(void)
{
	int a = 100;
	int b = 101;
	struct num_t mynum = {
		.a = 100,
		.b = 200,
		.c = 300,
	};
	struct num_t *np = &mynum;
	pr_x("%d\n", min(a, b));
	pr_x("adsfasdf\n");
	pr_x("%d, %d, %d\n", mynum.a, mynum.b, mynum.c);
	printf("sizeof(unsigned short)=%d\n", sizeof(unsigned short));
	printf("sizeof(np->abc)=%d\n", sizeof(np->abc));
	printf("sizeof(mynum.abc)=%d\n", sizeof(mynum.abc));
	printf("sizeof(unsigned long)=%d\n", sizeof(unsigned long));
	printf("main: %p\n", (void *)main);
	printf("func0: %p\n", (void *)func0);
	printf("sizeof(minsize): %d\n", sizeof(struct minsize));
	printf("sizeof A(%d), B(%d), C(%d), D(%d)\n",
			sizeof(struct A), sizeof(struct B), sizeof(struct C), sizeof(struct D));
	return 0;
}
