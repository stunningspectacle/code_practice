#include <stdio.h>
void main(int argc, char *argv[])
{
	char **p = argv;
	p += argc;
	p++; // skip NULL
	while (*p) {
		printf("%s\n", *p);
		p++;
	}
}

