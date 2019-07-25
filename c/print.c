#include <stdio.h>
#include <unistd.h>

unsigned short values[] = {
	0x4c,0x55,0x49,0x44,
	0x3a,0x30,0x30,0x37,
	0x33,0x30,0x30,0x30,
	0x39,0x30,0x30,0x30,
	0x38,0x30,0x30,0x30,
	0x32,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0 };

void test_printf_char()
{
	int i;
	char *c = (char *)values;

	for (i = 0; i < sizeof(values); i += 8) {
		printf("%c,%c,%c,%c,%c,%c,%c,%c\n",
				c[i + 0],
				c[i + 1],
				c[i + 2],
				c[i + 3],
				c[i + 4],
				c[i + 5],
				c[i + 6],
				c[i + 7]);
	}
}

void test0()
{
	unsigned long long time = 0x1234567887654321;
	char str[] = "aaabbbba\n";

	write(STDIN_FILENO, str, sizeof(str));
	printf("time=0x%llx\n", time);
}

void main(void)
{
	test_printf_char();
}
