#include <stdio.h>
#include <stdarg.h>

void myprint_valist(va_list ap)
{
	char *p = NULL;

	while (p = va_arg(ap, char *))
		printf("%s\n", p);
}

void myprint_str(int n, ...)
{
	va_list ap;

	va_start(ap, n);
	myprint_valist(ap);
	va_end(ap);
}

void myprint(const char *fmt, ...)
{
	va_list args;
	const char *p = fmt;
	char c;
	int i;

	va_start(args, fmt);
	printf("sizeof(args): %ld,  fmt@%p, args@%p\n",
			sizeof(args), &fmt, args);

	for (p = fmt; *p; p++) {
		switch (*p) {
		case 'c':
			c = va_arg(args, int);
			printf("c: %c\n", c);
			break;
		case 'd':
			i = va_arg(args, int);
			printf("d: %d\n", i);
			break;
		default:
			break;
		}
	}
	va_end(args);
}

int main(void)
{
	myprint("cdcd", 'x', 0x123, 'y', 0x456);

	myprint("cd", 'x', 123);

	myprint_str(0, "aa", "bb", "cc", "dd", "ee", "ff");

	return 0;
}



