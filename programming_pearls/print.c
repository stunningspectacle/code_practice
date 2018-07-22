char *str = "Hello World!\n";

void print() {
	asm volatile (
		"movl $13, %%edx\n\t"
		"mov %0, %%rcx\n\t"
		"movl $0, %%ebx\n\t"
		"movl $4, %%eax\n\t"
		"int $0x80\n\t"
		::"r"(str));
		//:"%eax", "%edx", "%rcx", "%ebx");
}

void exit() {
	asm volatile (
		"movl $1, %eax \n\t"
		"movl $42, %ebx \n\t"
		"int $0x80 \n\t");
}

void nomain()
{
	print();
	exit();
}
			
