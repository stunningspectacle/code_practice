#include <stdio.h>

char *get_cpuid();
int myadd(int, int);

void main() 
{
	int res;
	//char *cpuid;


	printf("msg from main\n");
	res = myadd(123, 235);
	printf("result is %d\n", res);

	printf("cpuid: %s\n", get_cpuid());
}
