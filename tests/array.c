int akk[] = {10, 20, 30, 40, 50 };

void main()
{
	int b = 0;
	int i;

	for (i = 0; i < sizeof(akk)/sizeof(akk[0]); i++) {
		printf("This is %d\n", akk[i]);
		b += akk[i];
	}

	exit(0);
}
