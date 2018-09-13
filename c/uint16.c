#include <stdio.h>


typedef unsigned short int uint16_t; 

int main()
{
	uint16_t n0 = 100;

	int a = 15;
	int b = 10;

	int c = b - a;

	n0 += c;

	printf("%d\n", n0);
}
