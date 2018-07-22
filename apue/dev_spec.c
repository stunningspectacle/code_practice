#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc , char * argv[])
{
	int i, fd, len;
	struct stat st;
	for (i = 0; i < argc; i++) {
		if (lstat(argv[i], &st) == -1) {
			printf("stat failed");
			continue;
		}
		printf("%s: dev = %d/%d",
				argv[i], major(st.st_dev), minor(st.st_dev));

		if (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)) {
			printf(" %s rdev = %d/%d",
					(S_ISDIR(st.st_mode)) ? "(dir)" : "(char)",
					major(st.st_rdev), minor(st.st_rdev));
		}
		printf(" size: %ld", st.st_size);
		printf("\n");
	}

	return 0;
}

