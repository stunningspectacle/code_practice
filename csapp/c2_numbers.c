#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

void signed_num(void)
{
	char n0 = 0;

	n0 = 1 << 7 | 1;
	printf("n0 = %d\n", n0);
}

void print_tmin(void)
{
	int tmin = 0x80000000;
	int tmin_plus1 = 0x80000000 + 1;

	printf("tmin = %d, -tmin=%d\n", tmin, -tmin);
	printf("tmin_plus1 = %d, -tmin_plus1=%d\n", tmin_plus1, -tmin_plus1);
}

void print_num(void)
{
	int a = -1;
	int32_t b = -22;

	printf("int = %d, unsigned int = %u, hex = %x\n", a, a, a);

	printf("int32 = %" PRId32 ", uint32 = %" PRIu32 ", hex = %x\n", b, b, b);
}

/* When compared a signed number and unsigned number, the signed
 * number will be cast to unsigned, so -1 means 0xffffffff
 */
static void nonintuitive_compare()
{
	if (-1 < 0u)
		printf("-1 < 0u\n");
	else
		printf("-1 > 0u\n");
}

static void add()
{
	int a = -0x77777777;
	int b = -0x66666666;

	printf("a: 0x%x, b: 0x%x, a+b: 0x%x\n", a, b, a+b);
}

int main(int argc, char *argv[])
{
	signed_num();
	print_num();
	add();
	nonintuitive_compare();
	printf("sizeof(-1)=%ld\n", sizeof(-1));
	printf("sizeof(0)=%ld\n", sizeof(0));
	print_tmin();

	return 0;
}

