#include <stdio.h>

int glb_a = 1223;
int glb_b = 38;

void my_exit()
{
	asm(	"movl $1, %eax	\n\t"
			"movl $123, %ebx \n\t"
			"int $0x80		\n\t"
	   );
}

void use_glb()
{
	glb_a = 10;
	glb_b = 20;

	asm(	"pusha\n\t"
			"movl glb_a, %eax	\n\t"
			"addl glb_b, %eax	\n\t"
			"movl %eax, glb_b	\n\t"
			"popa\n\t"
	   );

	printf("glb_b = %d\n", glb_b);
}

void my_add() {
	int data1 = 100;
	int data2 = 200;
	printf("before add: %d, %d\n", data1, data2);
	
	asm(	"imull %%eax, %%ebx \n\t"
			"movl %%ebx, %%eax \n\t"
			: "=a"(data2)
			: "a"(data1), "b"(data2));
	printf("After add: %d, %d\n", data1, data2);
}

void copy_str() {
	char str1[30] = { "This is my string" };
	char str2[30] = { "not copied yet" };
	int length = 30;
	printf("%s, %s\n", str1, str2);

	asm volatile (
			"cld\n\t"
			"rep movsb\n\t"
			: // no output, because is already in input part: "D"(str2)
			: "S"(str1), "D"(str2), "c"(length));

	printf("%s, %s\n", str1, str2);
}

void place_holder() {
	int a = 123;
	int b = 456;
	int c = 0;

	printf("%d, %d, %d\n", a, b, c);
	asm (	"imull %1, %2\n\t"
			"movl %2, %0\n\t"
			: "=r"(c)
			: "r"(a), "r"(b));

	printf("%d, %d, %d\n", a, b, c);
}

void same_place_holder() {
	int a = 123;
	int b = 456;

	printf("%d, %d\n", a, b);

	asm (	"imull %1, %2\n\t"
			: "=r"(b)
			: "r"(a), "0"(b)); // use "0" to reference place holder "0"

	printf("%d, %d\n", a, b);
}

void named_place_holder() {
	int a = 1111;
	int b = 2222;

	printf("%d, %d\n", a, b);
	asm (	"imull %[value1], %[value2]\n\t"
			: [value2]"=r" (b)
			: [value1]"r"(a), "0"(b));

	printf("%d, %d\n", a, b);
}

void changed_register() {
	int a = 123;
	int b = 456;

	printf("%d, %d\n", a, b);
	asm (	"movl %1, %%eax\n\t"
			"addl %%eax, %0\n\t"
			: "=r"(b)
			: "r"(a), "0"(b)
			: "%eax");
	printf("%d, %d\n", a, b);
}

void use_memory() {
	int a = 1235;
	int b = 5;
	int result = 0;

	printf("%d, %d, %d\n", a, b, result);
	asm (	"divl %2\n\t"
			"movl %%eax, %0\n\t"
			: "=m"(result)
			: "a"(a), "m"(b));
	printf("%d, %d, %d\n", a, b, result);
}

// return greater number
int use_label(int a, int b) {
	int result;

	asm (	"cmpl %1, %2\n\t"
			"jg greater\n\t"
			"movl %1, %0\n\t"
			"jmp end\n\t"
			"greater:\n\t"
			"movl %2, %0\n\t"
			"end:\n\t"
			: "=m"(result)
			: "r"(a), "r"(b));
	return result;
}

int use_local_label(int a, int b) {
	int result;

	asm (	"cmpl %1, %2\n\t"
			"jg 0f\n\t"
			"movl %1, %0\n\t"
			"jmp 1f\n\t"
			"0:\n\t"
			"movl %2, %0\n\t"
			"1:\n\t"
			: "=m"(result)
			: "r"(a), "r"(b));
	return result;
}

#define NUM 10
int use_imm(unsigned char value, unsigned char port) {
	__asm__ __volatile__(
			"mov %0, %%eax\n\t"
			:
			:"i"(NUM)
			:"eax");
}

unsigned char inb_outb(unsigned char value, unsigned char port)
{
	unsigned char ret = 0;

	__asm__ __volatile__(
			"outb %%al, %%dx\n\t"
			:
			:"a"(value), "d"(port));

	__asm__ __volatile__(
			"xor %%eax, %%eax\n\t"
			"inb %%dx, %%al\n\t"
			: "=a"(ret)
			: "d"(port));
}

int main(int argc, char *argv[]) {
	int result;
	printf("this is the first line\n");

	result = use_local_label(atoi(argv[1]), atoi(argv[2]));
	printf("%d is greater\n", result);
	
}
	

