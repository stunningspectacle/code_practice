#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void dump(int array[], int size) {
	int i;

	for (i = 0; i < size; i++) {
		printf("%3d ", array[i]);
		if (i != 0 && i % 40 == 0) {
			printf("\n");
		}
	}
	if (--i % 50 != 0) {
		printf("\n");
	}
	printf("\n");
}

void insertSort(int array[], int size)
{
	int i, j;
	int tmp;

	for (i = 1; i < size; i++) {
		tmp = array[i];
		for (j = i; j > 0; j--) {
			if (array[j - 1] > tmp) {
				array[j] = array[j - 1];
			} else {
				break;
			}
		}
		array[j] = tmp;
	}
}

void swap(int *a, int *b) {
	int tmp = *a;

	*a = *b;
	*b = tmp;
}

int medium3(int array[], int left, int right)
{
	int center = (left + right) / 2;

	if (array[left] > array[center])
		swap(&array[left], &array[center]);
	if (array[left] > array[right])
		swap(&array[left], &array[right]);
	if (array[center] > array[right])
		swap(&array[center], &array[right]);

	swap(&array[center], &array[right - 1]);

	return array[right - 1];
}

#define CUTOFF 20
void doQuickSort(int array[], int left, int right)
{
	int i, j, pivot;

	if (left + CUTOFF <= right) {
		pivot = medium3(array, left, right);
		i = left;
		j = right - 1;

		for (;;) {
			while (array[++i] < pivot) { }
			while (array[--j] > pivot) { }

			if (i < j) {
				swap(&array[i], &array[j]);
			} else {
				break;
			}
		}
		swap(&array[i], &array[right - 1]);

		doQuickSort(array, left, i - 1);
		doQuickSort(array, i + 1, right);
	} else {
		insertSort(array + left, right - left + 1);
	}
}

void doQuickSort2(int array[], int left, int right)
{
	int i, j, pivot;

	if (right - left > 10) {
		pivot = array[right];
		i = left;
		j = right - 1;

		for (;;) {
			while (array[i] < pivot) { i++; }
			while (array[j] > pivot) { j--; }

			if (i < j)
				swap(&array[i++], &array[j--]);
			if (i > j)
				break;
		}
		swap(&array[i], &array[right]);

		doQuickSort2(array, left, i - 1);
		doQuickSort2(array, i + 1, right);
	} else {
		insertSort(array + left, right - left + 1);
	}
}

void quickSort(int array[], int size)
{
	doQuickSort2(array, 0, size - 1);
}

void merge(int array[], int tmpArray[], int leftStart, int rightStart, int rightEnd)
{
	int leftEnd;
	int left, right;
	int i;

	leftEnd = rightStart - 1;
	left = leftStart;
	right = rightStart;

	i = leftStart;

	while (left <= leftEnd && right <= rightEnd) {
		if (array[left] < array[right])
			tmpArray[i++] = array[left++];
		else
			tmpArray[i++] = array[right++];
	}
	while (left <= leftEnd)
		tmpArray[i++] = array[left++];
	while (right <= rightEnd)
		tmpArray[i++] = array[right++];

	for (i = leftStart; i <= rightEnd; i++) {
		array[i] = tmpArray[i];
	}
}

void doMergeSort(int array[], int tmpArray[], int left, int right)
{
	int center;

	center = (left + right) / 2;
	if (left < right) {
		doMergeSort(array, tmpArray, left, center);
		doMergeSort(array, tmpArray, center + 1, right);
		merge(array, tmpArray, left, center + 1, right);
	}
}

void mergeSort(int array[], int size) {
	int *tmpArray;

	tmpArray = malloc(size * sizeof(int));
	doMergeSort(array, tmpArray, 0, size - 1);
}

void shellSort(int array[], int size)
{
	int i, j, increment;
	int tmp;

	for (increment = size/2; increment > 0; increment /= 2) {
		for (i = increment; i < size; i++) {
			tmp = array[i];
			for (j = i; j >= increment; j -= increment) {
				if (tmp < array[j-increment]) {
					array[j] = array[j-increment];
				} else {
					break;
				}
			}
			array[j] = tmp;
		}
		printf("After increment = %d:\n", increment);
		dump(array, size);
	}
}

/* Something is wrong ... */
void shellSort1(int array[], int size)
{
	int i, j, increment;
	int tmp;

	for (increment = size/2; increment > 0; increment /= 2) {
		for (i = 0; i + increment < size; i++) {
			tmp = array[i];
			for (j = i; j + increment < size; j += increment) {
				if (tmp > array[j+increment]) {
					array[j] = array[j+increment];
				} else {
					break;
				}
			}
			array[j] = tmp;
		}
		printf("After increment = %d:\n", increment);
		dump(array, size);
	}
}

int checkArray(int array[], int size) {
	int i;

	for (i = 0; (i + 1) < size; i++) {
		if (array[i] > array[i+1]) {
			printf("array is not in order, array[%d] > array[%d]\n", i, i+1);
			return 1;
		}
	}

	printf("array is in order\n");

	return 0;
}

void main(int argc, char *argv[]) {
	FILE *file;
	int i, count;
	int *array;
	char buf[16];

	if (argc != 3) {
		printf("usage: %s file count\n", argv[0]);
		return;
	}

	count = atoi(argv[2]);

	file = fopen(argv[1], "r");	
	if (!file) {
		perror("Open file failed");
		return;
	}
	array = malloc(count * sizeof(int));
	for (i = 0; i < count && fgets(buf, sizeof(buf), file); i++)
		array[i] = atoi(buf);

	checkArray(array, count);
	dump(array, count);
	//shellSort(array, count);
	//mergeSort(array, count);
	//insertSort(array, count);
	quickSort(array, count);
	checkArray(array, count);
	dump(array, count);
}

