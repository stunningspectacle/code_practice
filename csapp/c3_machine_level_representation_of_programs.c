#include <stdio.h>
#include <inttypes.h>

static int global0 = 0;

/* use the gcc built-in __int128 type */
typedef __int128 uint128_t;

static inline void uint128_mul(uint128_t *dest, uint64_t s0, uint64_t s1)
{
	*dest = s0 * (uint128_t)s1;
}

static void asm0(void)
{
	int local0 = 0;
	int local1 = 0;

	uint64_t flags = 0;

#if 0
	__asm__ volatile (
			"push %rax\n\t"
			"mov $0xABCD, %rax\n\t"
			"mov %rax, global0\n\t"
			"pop %rax\n\t");

	printf("global0 = 0x%x\n", global0);
#endif

	__asm__ __volatile__ (
			"movl $0xccdd, (%1)\n\t"
			"mov (%1), %0\n\t"
			: "=r"(local1)
			: "r"(&local0));

	printf("local0 = 0x%x\n", local0);
	printf("local1 = 0x%x\n", local1);

	__asm__ volatile (
			"pushf\n\t"
			"pop %0\n\t"
			: "=m"(flags)
			:
			);
	printf("flags = 0x%lx\n", flags);
}

int main(void)
{
	uint128_t s;
	uint64_t a = 0xaaaaaaaaaaaaaaaa;
	uint64_t b = 0xbbbbbbbbbbbbbbbb;
	uint64_t *p;

	asm0();

	uint128_mul(&s, a, b);
	p = (uint64_t *)&s;
	printf("0x%lx * 0x%lx = 0x%lx%lx\n", a, b, p[1], p[0]);
}
