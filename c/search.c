#include <stdio.h>

int search(const char *src, const char *target)
{
	const char *tmp_src = src;
	const char *tmp_target = target;

	while (*tmp_src && *tmp_target) {
		if (*tmp_src != *tmp_target) {
			tmp_src++;
			tmp_target = target;
			continue;
		}
		tmp_src++;
		tmp_target++;
	}

	if (*tmp_target)
		return -1;
	return (tmp_src - src - (tmp_target - target));
}

#define CONTIG_LEN 3
#define TARGET_VAL 0
int test[] = { 1 , 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0};
int search_contig_area(int *array, int array_size, int val)
{
	int offset;
	int contig_end;

	offset = 0;
	contig_end = offset + CONTIG_LEN - 1;
	for (; offset < array_size; offset++) {
		if (array[offset] != TARGET_VAL) {
			contig_end = offset + CONTIG_LEN;
			continue;
		}
		if (offset == contig_end)
			return offset - CONTIG_LEN + 1;
	}

	return -1;
}

int main(int argc, char *argv[]) {
	int ret;

	ret = search_contig_area(test, sizeof(test)/sizeof(test[0]), TARGET_VAL);

	if (ret < 0) {
		printf("Cannot find\n");
	} else {
		printf("Found, index = %d\n", ret);
	}

	return 0;
}

