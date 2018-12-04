#include <stdio.h>
#define PTR_SUB(a, b) ((char *)a - (char *)b)

#pragma pack(1)
struct test1 {
	char a;
	int b;
	char c;
	int d;
};

struct test3 {
	char a;
	int d;
	char b;
	char c;
};
#pragma pack()

struct test2 {
	char a;
	int b;
	char c;
	int d;
};
struct test4 {
	char a;
	char b;
	int d;
	char c;
};
struct test5 {
	char a;
	char b;
	char c;
	char d;
	char e;
	int f;
};
struct test6 {
	char a1;
	char a2;
	char a3;
	char a4;
	char a5;
	char a6;
};

void main(void)
{
	struct test4 test4, *t4;

	t4 = &test4;

	printf("sizeof(test1)=%d\n", sizeof(struct test1));
	printf("sizeof(test2)=%d\n", sizeof(struct test2));
	printf("sizeof(test3)=%d\n", sizeof(struct test3));
	printf("sizeof(test4)=%d\n", sizeof(struct test4));
	printf("sizeof(test5)=%d\n", sizeof(struct test5));
	printf("%d, %d, %d, %d\n",
			PTR_SUB(&t4->a, t4),
			PTR_SUB(&t4->d, t4),
			PTR_SUB(&t4->b, t4),
			PTR_SUB(&t4->c, t4));
	printf("sizeof(test6)=%d\n", sizeof(struct test6));
}
