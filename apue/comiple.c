#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

int main(int argc, char *argv[])
{
	int a = sizeof(char[10]);

	printf("a=%d\n", a);
	BUILD_BUG_ON(0);

	return 0;
}

