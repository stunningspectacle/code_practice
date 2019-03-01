#include <stdio.h>

typedef void (*func_t)();

#define TEST_INIT(fn, n) \
	func_t fn##n __attribute__((__used__, section("my_section"))) = fn;

void test0()
{
	printf("This is %s\n", __func__);
}
TEST_INIT(test0, 0)

void test1()
{
	printf("This is %s\n", __func__);
}
TEST_INIT(test1, 1)

extern void * __start_my_section;
extern void * __stop_my_section;

void main()
{
	printf("start %p, stop %p\n", __start_my_section, __stop_my_section);
	func_t *start = (func_t *)&__start_my_section;
	func_t *stop = (func_t *)&__stop_my_section;
	printf("start %p, stop %p\n", start, stop);

	while (start < stop) {
		printf("func @%p\n", *start);
		(*start)();
		start++;
	}
}
