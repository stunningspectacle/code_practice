#include <stdio.h>

int main(int argc, char *argv[])
{
	int i;
	char **p;
	char ***p0;

	p0 = &argv;

	printf("&argc=%p\n", &argc);
	printf("&argv=%p\n", p0);
	printf("argv=%p\n", *p0);

	p = argv;
	for (i = 0; i < argc; i++) {
		printf("%p: %s\n", p, *p);
		p++;
	}

	p++;

	while (*p) {
		printf("%p: %s\n", *p);
		p++;
	}

	return 0;
}

