#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int doGetLargestSubNLogN(int array[], int left, int right)
{
	int maxLeftSum, maxRightSum;
	int maxLeftBorderSum, maxRightBorderSum;
	int leftBorderSum, rightBorderSum;
	int center;
	int i;
	int max;

	if (left == right) {
		if (array[left] > 0)
			return array[left];
		return 0;
	}

	center = (left + right) / 2;
	maxLeftSum = doGetLargestSubNLogN(array, left, center);
	maxRightSum = doGetLargestSubNLogN(array, center + 1, right);
	
	leftBorderSum = maxLeftBorderSum = 0;
	for (i = center; i >= left; i--) {
		leftBorderSum += array[i];
		if (leftBorderSum > maxLeftBorderSum)
			maxLeftBorderSum = leftBorderSum;
	}

	rightBorderSum = maxRightBorderSum = 0;
	for (i = center + 1; i <= right; i++) {
		rightBorderSum += array[i];
		if (rightBorderSum > maxRightBorderSum)
			maxRightBorderSum = rightBorderSum;
	}

	max = maxLeftSum > maxRightSum ? maxLeftSum : maxRightSum;
	max = max > (maxLeftBorderSum + maxRightBorderSum) ?
		max : (maxLeftBorderSum + maxRightBorderSum);

	return max;
}

int getLargestSubNLogN(int array[], int size)
{
	int max;

	max = doGetLargestSubNLogN(array, 0, size - 1);

	printf("%s: maxSum = %d\n", __func__, max);
}

int getLargestSubN2(int array[], int size)
{
	int i, j, k;
	int maxSum = 0, maxIndex, maxLen;
	int sum;

	for (i = 0; i < size; i++) {
		sum = 0;
		for (j = i; j < size; j++) {
			sum += array[j];
			if (sum > maxSum) {
				maxIndex = i;
				maxLen = j - i + 1;
				maxSum = sum;
			} else if (sum <= 0) {
				break;
			}
		}
	}
	printf("maxSum=%d, maxIndex=%d, maxlen=%d\n", maxSum, maxIndex, maxLen);
	for (i = 0; i < maxLen; i++)
		printf("%d ", array[maxIndex + i]);
	printf("\n");
}

int getLargestSubN(int array[], int size)
{
	int i, j, k;
	int maxSum = 0, maxIndex = -1, maxLen = 0;
	int sum = 0, index = -1, len = 0;

	for (i = 0; i < size; i++) {
		sum += array[i];
		len++;
		if (index == -1)
			index = i;
		if (sum <= 0) {
			sum = 0;
			len = 0;
			index = -1;
		} else if (sum > maxSum) {
			maxLen = len;
			maxIndex = index;
			maxSum = sum;
		}
	}
	printf("maxSum=%d, maxIndex=%d, maxlen=%d\n", maxSum, maxIndex, maxLen);
	for (i = 0; i < maxLen; i++)
		printf("%d ", array[maxIndex + i]);
	printf("\n");
}

void dump(int array[], int size) {
	int i;

	for (i = 0; i < size; i++)
		printf("%d ", array[i]);
	printf("\n");
}

void main(int argc, char *argv[]) {
	struct heap *h;
	FILE *file;
	char buf[8];
	int size, num, i = 0;
	int *array;

	if (argc != 3) {
		printf("Usage: %s file count\n", argv[0]);
		return;
	}

	file = fopen(argv[1], "r");
	if (!file) {
		perror("open file failed");
		return;
	}

	size = atoi(argv[2]);
	array = malloc(num * sizeof(int));
	while (fgets(buf, sizeof(buf), file)) {
		num = atoi(buf);
		array[i++] = num;
	}
	dump(array, size);
	getLargestSubN2(array, size);
	getLargestSubN(array, size);
	getLargestSubNLogN(array, size);
}
