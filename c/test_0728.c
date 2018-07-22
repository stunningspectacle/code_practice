#include <stdio.h>

int array[10] = {
	[5] = 10,
	[6] = 8,
};


int main() {
	printf("sizeof(array) = %i\n", sizeof(array)/sizeof(array[0]));
}
