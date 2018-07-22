#include <stdio.h>

int test_array0[10] = {
	[2] = 2,
	[3] = 3,
	[8] = 8,
};

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

int main() 
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
