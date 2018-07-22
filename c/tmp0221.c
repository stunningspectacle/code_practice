#include <stdio.h>

int main(int argc, char *argv[])
{
	int i;
	char **stack = argv;

		
	for (i = 0; i < argc; i++) {
		stack++;
	}

	stack++;

	while (*stack) {
		printf("%s\n", *stack);
		stack++;
	}
}
