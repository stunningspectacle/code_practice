#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

static int global0 = 0;

/* use the gcc built-in __int128 type */
typedef __int128 uint128_t;

static inline void uint128_mul(uint128_t *dest, uint64_t s0, uint64_t s1)
{
	*dest = s0 * (uint128_t)s1;
}

static void pc_update(void)
{
	void *pc;

	__asm__ __volatile__ (
			"pushq %%rbp\n\t"
			"popq %0\n\t"
			: "=m"(pc)
			:
			);

	printf("pc: %p\n", pc);
}

static int add_div(short x, short y, short z)
{
#if 0
	__asm__ __volatile__(
			//"leaq (%rdi, %rsi, 1), %rax\n\t"
			"mov -4(%rsp), %ax\n\t"
			"addw -8(%rsp), %ax\n\t"
			"addw -12(%rsp), %ax\n\t"
			//"addq %rdx, %rax\n\t"
			);
#endif

	__asm__ __volatile__ (
			"movswq %0, %%rax\n\t"
			"add %1, %%ax\n\t"
			"add %2, %%ax\n\t"
			:
			: "m"(x), "m"(y), "m"(z));
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

int get_polarity(short x)
{
	short val = 1;
	while (x != 0) {
		val ^= x;
		x >>= 1;
	}
	return val & 1;
}

int get_polarity_simple(short x)
{
	int count = 0;
	while (x != 0) {
		if (x & 1)
			count++;
		x >>= 1;
	}
	if (count % 2 == 0)
		return 1;
	return 0;
}

long reverse_bit(long x)
{
	int i;
	long val = 0;
	for (i = 64; i != 0; i--) {
		val = (val << 1) | (x & 0x1);
		x >>= 1;
	}

	return val;
}

void jump_table(int x)
{
	void *jt[] = { &&l0, &&l1, &&l2, &&l3, &&l4, &&l5, &&l6 };
	if (x < 0 || x > 6)
		goto no;

	goto *jt[x];

l0:
	printf("this is l0\n");
	goto done;
l1:
	printf("this is l1\n");
	goto done;
l2:
	printf("this is l2\n");
	goto done;
l3:
	printf("this is l3\n");
	goto done;
l4:
	printf("this is l4\n");
	goto done;
l5:
	printf("this is l5\n");
	goto done;
l6:
	printf("this is l6\n");
	goto done;
no:
	printf("not support\n");

done:
	printf("done\n");
}

typedef int row3_t[3];
int typedef_array(void)
{
	row3_t arr[10];
	int a = arr[5][2];

	return a;
}

int main(int argc, char *argv[])
{
	typedef_array();

	return 0;
}
