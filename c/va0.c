#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

void myprintf0(const char *fmt, ...)
{
	va_list ap;
	int a;
	char c;
	long l;

	va_start(ap, fmt);

	while (*fmt) {
		switch (*fmt++) {
		case 'd':
			a = va_arg(ap, int);
			break;
		case 'c':
			c = va_arg(ap, char);
			break;
		case 'l':
			l = va_arg(ap, long);
			break;
		default:
			break;
		}
	}
}

void myprintf(const char *fmt, ...) 
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

int main(void)
{
	myprintf("%s: This is a test: %d\n", __func__, __LINE__);

	return 0;
}

