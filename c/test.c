#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

extern char **environ;
typedef unsigned short uint16_t;

struct testA {
	unsigned int type;
	unsigned int len;
	unsigned flags;
	unsigned char data[];
};

#define DEBUG(fmt, args...) printf(fmt, ##args)
#define ASSERT_EQ(a, b) \
do {\
	if ((a) != (b)) {\
		DEBUG("Error, " #a " != " #b " at %s() line %d\n", __func__, __LINE__);\
	}\
} while (0)

int myffs(unsigned long data) {
	int count = 0;
	if (data == 0)
		return -1;

	if ((data & 0xffff) == 0) {
		count += 16;
		data >>= 16;
	}
	if ((data & 0xff) == 0) {
		count += 8;
		data >>= 8;
	}
	if ((data & 0xf) == 0) {
		count += 4;
		data >>= 4;
	}
	if ((data & 0x3) == 0) {
		count += 2;
		data >>= 2;
	}
	if ((data & 0x1) == 0) {
		count += 1;
	}
	return count;
}

int try(int a, int b, int c) {
	printf("try %d, %d, %d\n", a, b, c);
	return 0;
}

void myfree(void *p) {
	free(p);
}

int try1(void)
{
#define SIZE 10
	int i;
	char *abc = malloc(SIZE);
	for (i = 0; i < SIZE; i++)
		abc[i] = 'a' + i;

	for (i = 0; i < SIZE * 2; i++)
		printf("0x%x ", abc[i]);
	printf("\n");
	printf("abc=%p\n", abc);
	printf("abc=%s\n", abc);
	myfree(abc);
	printf("abc=%p\n", abc);
	printf("abc=%s\n", abc);

	return 0;
}

int myprint(int num)
{
	if (num < 0) {
		printf("-");
		num = -num;
	}
	if (num >= 10) {
		myprint(num / 10);
	}
	printf("%c", (num % 10) + '0');
}

long myatol(char *s) {
	long value = 0;
	char *p = s;
	int minus = 0;

	while (*p && *p != '-' && !(*p >= '0' && *p <= '9'))
		p++;

	if (*p == '-') {
		minus = 1;
		p++;
	}

	while (*p >= '0' && *p <= '9') {
		value = value * 10 + (*p - '0');
		p++;
	}

	if (minus)
		value = -value;
	return value;
}

void test_div_zero() {
	int num1 = 1000;
	int num2 = 4;

	asm volatile (
			"divb %%bl\n\t"
			"movl %%eax, %0\n\t"
			:"=b"(num2)
			:"a"(num1), "0"(num2));

	printf("a / b = %d\n", num2);
}

void test_div_zero0() {
	volatile int a = 100;
	volatile int b = 0;
	int c = 0;

	c = a / b;
}

void test_div_zero1() {
	asm ("int $16\n\t");
}

int my_test_div_zero() {
	char *s;
	int i = 0;
	int j = 0, k = 0;
	int array[10];

	for (i = 0; i < 20; i++) {
		j++;
		k++;
		array[i] = i;
	}
	test_div_zero();

	exit(123);
}

void test_uint() {
	uint16_t a = 100;
	uint16_t c = 90;
	int delta;

	uint16_t b;

	delta = c - a;
	b = a + delta;
	printf("%u, %d\n", b, delta);
}

void sizeof_long()
{
	printf("sizeof(unsigned short): %u\n", sizeof(unsigned short));
	printf("sizeof(unsigned int): %u\n", sizeof(unsigned int));
	printf("sizeof(long): %u\n", sizeof(unsigned long));
	printf("sizeof(long long): %u\n", sizeof(long long));
	printf("sizeof(double): %u\n", sizeof(double));
	printf("sizeof(long double): %u\n", sizeof(long double));
	printf("sizeof(void *): %u\n", sizeof(void *));
}

void test_typeof()
{
	int a;

	printf("%d\n", sizeof(typeof(a)));
}

void test_for(int num)
{
	int i;

	for (i = 2; i < num; i++)
		printf("i = %d\n", i);
	printf("Done\n", i);
}

void value_of_negtive(int val)
{
	unsigned int a = (unsigned int)val;

	printf("%d: 0x%x\n", val, a);
}

void main(int argc, char *argv[])
{
	value_of_negtive(atoi(argv[1]));
}
