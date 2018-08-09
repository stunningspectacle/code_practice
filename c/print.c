#include <stdio.h>
#include <unistd.h>

void main(void)
{
	unsigned long long time = 0x1234567887654321;
	char str[] = "aaabbbba\n";

	write(STDIN_FILENO, str, sizeof(str));
	printf("time=0x%llx\n", time);

}
