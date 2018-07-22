#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	char *newargs[] = {"my", "hello", "ok", "find", NULL };

	char *name;

	if (argc != 2) {
		return 0;
	}

	name = argv[1];

	execve(name, newargs, NULL);

	printf("done\n");
}
