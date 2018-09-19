#include <stdio.h>
#include <stdarg.h>

struct jj {
	int a;
	int b;
	int c;
};

void myadd(int num, ...)
{
	va_list ap;
	int sum = 0;

	va_start(ap, num);
	while (num--) {
		sum += va_arg(ap, int);
	}
	va_end(ap);

	printf("%d\n", sum);
}

void myprintf(int abc, char *fmt, ...)
{
	int i;
	struct jj j;
	va_list ap;

	printf("abc=%d\n", abc);
	va_start(ap, fmt);
	while (*fmt) {
		switch (*fmt) {
		case 'c':
			i = va_arg(ap, int);
			printf("%c\n", i);
			break;
		case 'i':
			i = va_arg(ap, int);
			printf("%d\n", i);
			break;
		case 'j':
			j = va_arg(ap, struct jj);
			printf("%d, %d, %d\n", j.a, j.b, j.c);
			break;
		}
		fmt++;
	}
	va_end(ap);
}

void main(void)
{
	struct jj j = { 111, 222, 333 };
	myprintf(100000, "ijc", 10, j, 'k');

	myadd(5, 1, 2, 3, 4, 5);
}


