#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int * getArray(int n) {
	int i;
	int *array = malloc(n * sizeof(int));

	for (i = 0; i < n; i++)
		array[i] = i;
	return array;
}

void move(int *array, int size, int distance) {
	int i, j;
	int tmp;

	for (i = 0; i < distance; i++) {
		tmp = array[i];
		for (j = i; j < size; j += distance) {
			if (j + distance < size)
				array[j] = array[j + distance];
			else
				array[j] = tmp;
		}
	}
}

void printArray(int *array, int size) {
	int i;
	int n = 0;
	char buf[150];

	for (i = 0; i < size; i++) {
		n += snprintf(buf + n,
			sizeof(buf) - n,
			"%d%s", array[i], i == (size - 1) ? "" : ", ");
		if (n >= 128) {
			buf[n] = '\0';
			printf("%s\n", buf);
			n = 0;
		}
	}
	if (n != 0)
		printf("%s\n", buf);
}

int main(int argc, char **argv) {
	int i;
	int n = 0;
	char arg[128];
	int size, distance;
	int *array;

	if (argc != 3) {
		for (i = 0; i < argc; i++) {
			n += snprintf(arg + n, sizeof(arg) - n,
					"%s%s",
					argv[i],
					i == (argc - 1) ? "" : ", ");
		}
		arg[n] = '\0';
		printf("%s\n", arg);
		return -1;
	}

	size = atoi(argv[1]);
	if (size <= 0) {
		printf("Wrong size!\n");
		return -1;
	}
	distance = atoi(argv[2]);
	if (distance <= 0 || distance >= size) {
		printf("Wrong distance!\n");
		return -1;
	}
	printf("Array size = %d, distance = %d\n", size, distance);

	array = getArray(size);
	printArray(array, size);
	move(array, size, distance);
	printArray(array, size);
}
