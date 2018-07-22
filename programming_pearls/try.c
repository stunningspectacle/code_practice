#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "stack.h"

#define END (~0)

int *test;
int xxxx_global_uninit;
__attribute__((weak)) void foo();
//extern void foo();
__attribute__((weak)) int pthread_create(pthread_t *thread,
		const pthread_attr_t *attr,
		void *(*start_routine) (void *),
		void *arg);

struct heapEntry {
	int n1;
	int n2;
};

struct heap {
	int capacity;
	int size;
	struct heapEntry elements[0];
};

struct ops {
	int isNumber;
	union {
		char c;
		int num;
	};
};

void dumpOps(struct ops *op) {
	if (op->isNumber) {
		printf("%d\n", op->num);
	} else {
		printf("%c\n", op->c);
	}
}

void change(int *p) {
	int a;

	if (p == NULL) {
		printf("yes, change it\n");
		p = &a;
	}
}

void swap(int *a, int *b)
{
	*a ^= *b ^= *a ^= *b;
}

int getLargestSub(int array, int size)
{
	int i, j;
	int maxSum = 0, maxIndex, maxLen;
	int sum;

	for (i = 0; i < size; i++) {
		sum = array[i];
		if (sum > maxSum) {
			maxIndex = i;
			maxSum = sum;
			maxLen = 1;
		}
		for (j = i + 1; j < size; j++) {
			sum += array[j];
			if (sum > maxSum) {
				maxSum = sum;
				maxLen = j - i;
			} else if (sum <= 0) {
				break;
			}
		}
	}
	printf("maxSum=%d:, maxIndex=%d, maxlen=%d\n", maxSum, maxIndex, maxLen);
	for (i = maxIndex; i < maxLen; i++)
		printf("%d ", array[i]);
	printf("\n");
}


void main(void) {
	int a = 100;
	int b = 200;

	printf("a = %d, b = %d\n", a, b);
	swap(&a, &b);
	printf("a = %d, b = %d\n", a, b);
}

void main(int argc, char *argv[]) {
	struct heap *h;
	FILE *file;
	char buf[8];
	int old = -1, num;

	if (argc != 3) {
		printf("Usage: %s file count\n", argv[0]);
		return;
	}

	file = fopen(argv[1], "r");
	if (!file) {
		perror("open file failed");
		return;
	}

	num = atoi(argv[2]);
	h = createHeap(num);
	while (fgets(buf, sizeof(buf), file)) {
		num = atoi(buf);
		/*
		if (insert(h, num) == NULL)
			break;
		*/
		h->elements[++h->size] = num;
	}

	buildHeap(h);

	if (checkHeap(h, 1) == 0) {
		printf("heap correct\n");
	} else {
		printf("heap incorrect\n");
		return;
	}
	//dump(h);

	while (h->size > 0) {
		//num = delMin(h);
		//num = delMin2(h);
		num = delMin3(h);
		printf("%d ", num);
		if (num < old) {
			printf("(----------------error, %d < %d---------------) ", num, old);
		}
		old = num;
	}
	printf("\n");
}
