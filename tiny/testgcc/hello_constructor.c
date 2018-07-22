#include <stdio.h>

void before_main(void) __attribute__((constructor));
void after_main(void) __attribute__((destructor));

void before_main(void)
{
	printf("say it before main in C\n");
}

void after_main(void)
{
	printf("say it after main in C\n");
}

int main()
{
	printf("Hello World!\n");
	return 0;
}
