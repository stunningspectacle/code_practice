#include <stdio.h>

#define err(num, args...) printf("abc" args)

#define LKF_REG_1 0x123
#define LKF_REG_2 0x111

#define COMM_REG_1 0x333
#define COMM_REG_2 0x444

#define NEW_REG_SET(proj) \
	unsigned int regs_##proj[2] = { \
	proj##_REG_1,			\
	proj##_REG_2,			\
	}


NEW_REG_SET(LKF);
NEW_REG_SET(COMM);

void common_handler(int irq_nr)
{
	printf("This is irq 0x%x\n", -irq_nr);
}

#define BUILD_IRQ(nr) \
	void IRQ##nr##_handler(void *data) { \
		__asm__ __volatile__(\
				"pushl $("#nr" - 256)\n\t" \
				"call common_handler\n\t" \
				"popl %eax\n\t"\
				);\
	}

#define BI(x, y) BUILD_IRQ(x##y)

#define BI16(x) \
	BI(x, 0) BI(x, 1) BI(x, 2) BI(x, 3) \
	BI(x, 4) BI(x, 5) BI(x, 6) BI(x, 7) \
	BI(x, 8) BI(x, 9) BI(x, A) BI(x, B) \
	BI(x, C) BI(x, D) BI(x, E) BI(x, F)

BI16(0x0)
BI16(0x1)
BI16(0x2)
BI16(0x3)
BI16(0x4)
BI16(0x5)
BI16(0x6)
BI16(0x7)
BI16(0x8)
BI16(0x9)
BI16(0xA)
BI16(0xB)
BI16(0xC)
BI16(0xD)
BI16(0xE)
BI16(0xF)


#define IRQ(x, y) IRQ##x##y##_handler

#define IRQ16(x) \
	IRQ(x, 0), IRQ(x, 1), IRQ(x, 2), IRQ(x, 3), \
	IRQ(x, 4), IRQ(x, 5), IRQ(x, 6), IRQ(x, 7), \
	IRQ(x, 8), IRQ(x, 9), IRQ(x, A), IRQ(x, B), \
	IRQ(x, C), IRQ(x, D), IRQ(x, E), IRQ(x, F)

typedef void (*irq_handler)(void *);

irq_handler irq_handlers[] = {
	IRQ16(0x0), IRQ16(0x1), IRQ16(0x2), IRQ16(0x3),
	IRQ16(0x4), IRQ16(0x5), IRQ16(0x6), IRQ16(0x7),
	IRQ16(0x8), IRQ16(0x9), IRQ16(0xA), IRQ16(0xB),
	IRQ16(0xC), IRQ16(0xD), IRQ16(0xE), IRQ16(0xF),
};

struct mong {
	char a;
	char b;
	int code __attribute__((packed));
};

void isLetter(char a)
{
	switch (a) {
	case 'a' ... 'z':
	case 'A' ... 'Z':
		printf("Yes\n");
		break;
	default:
		printf("No\n");
	}
}

int test(int a, int b, int c, int d)
{
	void *p = __builtin_return_address(2);
	printf("%p\n", p);

	return (a + b + c + d);
}

int use_test(int a, int b, int d, int e)
{
	void *args;
	void *ret;
	void *func = test;
	
	args = __builtin_apply_args();
	ret = __builtin_apply(func, args, 2);
	__builtin_return(ret);
}

void use_interrupt_handler(void)
{
	int i;

	int nr = sizeof(irq_handlers) / sizeof(irq_handlers[0]);

	printf("size = %u\n",  nr);

	for (i = 0; i < nr; i++) {
		irq_handlers[i](NULL);
	}
}

void main(void)
{
	use_interrupt_handler();
}

void usetest1(void)
{
	printf("add = %d\n", use_test(10, 20, 30, 40));
	printf("%p, %p, %p\n", (void *)test, (void *)use_test, (void *)main);
	isLetter(88);
}

