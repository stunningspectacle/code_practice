#include <stdio.h>

int main(int argc, char *argv[]) {
	int i = 0;
	int count = 0;
	char **stack_top = argv;

	//app path and args
	for (i = 0; i < argc; i++)
		stack_top++;

	//0x00000000
	stack_top++;

	while (*stack_top) {
		printf("%s\n", *stack_top);
		stack_top++;
		count++;
	}

	printf("%d env variables in total\n", count);
}

