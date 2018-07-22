#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *dirp;

	if (argc != 2) {
		printf("usage: %s name\n", argv[0]);
		return 0;
	}
	if ((dp = opendir(argv[1])) == NULL)
		printf("error!\n");
	while ((dirp = readdir(dp)))
		printf("%s\n", dirp->d_name);
	closedir(dp);

	return 0;

	DIR *dp;
	struct dirent *dirp;
	dp = opendir("xxxxxxx");
	while ((dirp = readdir(dp)))
}

