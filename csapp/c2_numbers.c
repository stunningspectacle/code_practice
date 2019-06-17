#include <sys/types.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	char n0 = 0;

	n0 = 1 << 7 | 1;
	printf("n0 = %d\n", n0);

	return 0;
}

