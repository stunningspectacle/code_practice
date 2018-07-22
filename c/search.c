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

int main(int argc, char *argv[]) {
	int ret;

	ret = search(argv[1], argv[2]);

	if (ret < 0) {
		printf("Cannot find\n");
	} else {
		printf("Found, index = %d\n", ret);
	}

	return 0;
}

