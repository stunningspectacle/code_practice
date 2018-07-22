#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char data[] = "abcdefghijklmn";

void reverse(char *s, int start, int end)
{
	int i;
	char tmp;
	int middle = (start + end) / 2;

	for (i = start; i <= middle; i++) {
		tmp = s[i];
		s[i] = s[end - (i - start)];
		s[end - (i - start)] = tmp;
	}
}

void move(char *s, int len, int num) {
	int start;
	char tmp;
	int curr, next; 
	int count = 0;

	for (start = 0; start < num; start++) {
		curr = start;
		tmp = s[curr];

		while (1) {
			count++;
			next = (curr + num) % len;
			if (next == start) {
				s[curr] = tmp;
				break;
			} else {
				s[curr] = s[next];
				curr = next;
			}
		}

		if (count == len)
			break;
	}
}

int main(int argc, char *argv[]) {
	int num;
	int len;

	if (argc != 2) {
		return;
	}

	num = atoi(argv[1]);
	len = strlen(data);

	printf("%s, num=%d\n", data, num);
	reverse(data, 0, num - 1);
	reverse(data, num, len - 1);
	reverse(data, 0, len - 1);
	//move(data, len, num);
	printf("%s, num=%d\n", data, num);
}
