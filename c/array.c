#include <stdio.h>

int test_array0[10] = {
	[2] = 2,
	[3] = 3,
	[8] = 8,
};

typedef struct zero_struct {
	int a;
	int b;
	int len;
	int data[];
} zero_struct_t;

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

void test1()
{
	printf("test_array0 1: %p\n", test_array0);
	printf("test_array0 2: %p\n", &test_array0[0]);
	printf("test_array0 3: %p\n", &test_array0);

	zero_struct_t zero1;
	printf("zero1 1: %p\n", zero1.data);
	printf("zero1 2: %p\n", &zero1.data);
	printf("zero1 3: %p\n", &zero1.data[0]);
}

void test0() 
{
	int i;
	
	int test_array1[10] = {
		[2] = 20,
		[3] = 30,
		[8] = 80,
	};

	int test_array2[10];

	for (i = 0; i < ARRAY_SIZE(test_array0); i++)
		printf("%d: %d\n", i, test_array0[i]);
	printf("\n");
	for (i = 0; i < ARRAY_SIZE(test_array1); i++)
		printf("%d: %d\n", i, test_array1[i]);
	printf("\n");
	for (i = 0; i < ARRAY_SIZE(test_array2); i++)
		printf("%d: %d\n", i, test_array2[i]);
	printf("\n");
}

void main()
{
	test1();
}
