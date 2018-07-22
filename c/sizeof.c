#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	float f;
	int a = 10;
	int b = 3;
	printf("sizeof(double)=%d\n", sizeof(double));
	printf("sizeof(long)=%d\n", sizeof(long));
	printf("sizeof(long long)=%d\n", sizeof(long long));
	f = (float)a / b;
	printf("%f\n", f);
}
