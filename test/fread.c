#include <stdio.h>

struct try {
	int a;
	char c;
};

void print() {
	struct try mytry;

	memset(&mytry, 0, sizeof(mytry));
	FILE * f = fopen("abc.txt", "r");

	while (fread(&mytry, sizeof(mytry), 1, f)) {
		printf("a = %d, c = %c\n", mytry.a, mytry.c);
	}

	fclose(f);
}

void create(void) {
	int i;
	struct try mytry;
	FILE * f = fopen("abc.txt", "r+");

	for (i = 0; i < 10; i++) {
		mytry.a = i;
		mytry.c = 'a' + i;
		fwrite(&mytry, sizeof(mytry), 1, f);
	}
	
	fclose(f);

	print();

}

int main(void) {
	int i;
	struct try mytry;
	FILE * f = fopen("abc.txt", "r+");

	create();

	while (fread(&mytry, sizeof(mytry), 1, f)) {
		mytry.a = 100;
		mytry.c = 'k';
		//fseek(f, -sizeof(mytry), SEEK_CUR);
		fwrite(&mytry, sizeof(mytry), 1, f);
	}

	fclose(f);

	print();

	return 0;
}



