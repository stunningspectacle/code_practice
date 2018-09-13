#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

//#define DEBUG

#ifdef DEBUG
#define debug_log(...) printf(__VA_ARGS__)
#else
#define debug_log(...) 
#endif

#define BUF_SIZE 128

void myprintf_vprintf(const char *fmt, ...)
{
	va_list ap;
	char *s, c;
	int d;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void myprintf(const char *fmt, ...)
{
	va_list ap;
	char buf[BUF_SIZE], c, *s;
	int d, len = 0;


	memset(buf, 0, BUF_SIZE);
	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt != '%') {
			len += snprintf(buf + len,
					BUF_SIZE - len, "%c", *fmt++);
			continue;
		}

		fmt++;
		switch (*fmt) {
		case 'c':
			c = (char)va_arg(ap, int);
			len += snprintf(buf + len, BUF_SIZE - len, "%c", c);
			debug_log("c: %c, buf: %s\n", c, buf);
			break;
		case 'd':
			d = va_arg(ap, int);
			len += snprintf(buf + len, BUF_SIZE - len, "%d", d);
			debug_log("d: %d, buf: %s\n", d, buf);
			break;
		case 's':
			s = va_arg(ap, char *);
			len += snprintf(buf + len, BUF_SIZE - len, "%s", s);
			debug_log("s: %s, buf: %s\n", s, buf);
			break;
		default:
			printf("Not support %c!\n", *fmt);
			break;
		}
		fmt++;
	}
	va_end(ap);

	printf("%s", buf);
}

void myvprintf(const char *prefix, const char *fmt, ...)
{
	char buf[BUF_SIZE];
	va_list ap;
	int len = 0;

	va_start(ap, fmt);
	memset(buf, 0, BUF_SIZE);

	len += snprintf(buf, BUF_SIZE, "%s", prefix);
	while (*fmt) {
		if (*fmt != '%') {
			len += snprintf(buf + len,
					BUF_SIZE - len, "%c", *fmt++);
			continue;
		}
		
		fmt++;
		switch (*fmt) {
		case 'c':
			len += vsnprintf(buf + len, BUF_SIZE - len, "%c", ap);
			debug_log("c - buf: %s\n", buf);
			break;
		case 'd':
			len += vsnprintf(buf + len, BUF_SIZE - len, "%d", ap);
			debug_log("d - buf: %s\n", buf);
			break;
		case 's':
			len += vsnprintf(buf + len, BUF_SIZE - len, "%s", ap);
			debug_log("s - buf: %s\n", buf);
			break;
		default:
			printf("Not supported %c\n", *fmt);
			break;
		}
		fmt++;
	}
	va_end(ap);

	printf("%s", buf);
}

void main()
{
	int a = 0;
	char c = 'k';

	myprintf_vprintf("%s: int = %d\n", __func__, a);

	a = 100;
	myprintf("%s: int = %d, c = %c, int = %d\n", __func__, a++, c, a);

	a = 200;
	myvprintf("vprintf:", "%s: int = %d, c = %c, int = %d\n",
			__func__, a++, c, a);
}
