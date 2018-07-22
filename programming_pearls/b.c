int shared = 1;
char global_notinit;

void swap(int *a, int *b) {
	*a ^= *b ^= *a ^= *b;
}
