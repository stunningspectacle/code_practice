static inline void barrier()
{
	__asm__ volatile ("" :::"memory");
}

int read32(unsigned int addr)
{
	return *(unsigned int *)addr; 
}

int read32_volatile(unsigned int addr1, unsigned int addr2)
{
	int i;
	volatile unsigned int value;
	value = *(const volatile unsigned int *)addr1;

//	barrier();

	for (i = 0; i < 100; i++)
		addr2 = i * i;

	return value;
}

