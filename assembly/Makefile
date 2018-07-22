TARGETS = cpuid cpuid_main cpuid_printf cpuid_printf_gcc sizetest data \
		  fpu movstring lods_stos func env inline_asm mainprog
all: $(TARGETS)

cpuid: cpuid.s
	gcc -g -c cpuid.s
	ld -o cpuid cpuid.o

cpuid_main: cpuid_main.s
	gcc -g -o cpuid_main cpuid_main.s

cpuid_printf: cpuid_printf.s
	as -gstabs -o cpuid_printf.o cpuid_printf.s
	ld --dynamic-linker /lib64/ld-linux-x86-64.so.2 -o cpuid_printf -lc cpuid_printf.o 

cpuid_printf_gcc: cpuid_printf_gcc.s
	gcc -g -o cpuid_printf_gcc cpuid_printf_gcc.s

sizetest: sizetest.s
	as -gstabs -o sizetest.o sizetest.s
	ld --dynamic-linker /lib64/ld-linux-x86-64.so.2 -o sizetest -lc sizetest.o 

data: data.s
	as -gstabs --32 -o data.o data.s
	ld -melf_i386 -o data data.o 

fpu: fpu.s
	as -gstabs --32 -o fpu.o fpu.s
	ld -melf_i386 -o fpu fpu.o 

movstring: movstring.s
	as -gstabs --32 -o movstring.o movstring.s
	ld -melf_i386 -o movstring movstring.o 

lods_stos: lods_stos.s
	as -gstabs --32 -o lods_stos.o lods_stos.s
	ld -melf_i386 -o lods_stos lods_stos.o 

func: func.s func1.s
	as -gstabs --32 -o func1.o func1.s
	as -gstabs --32 -o func.o func.s
	ld -melf_i386 -o func func.o func1.o
env: env.s
	as -gstabs --32 -o env.o env.s
	ld -melf_i386 -dynamic-linker /lib/ld-linux.so.2 -o env -lc env.o 

inline_asm: inline_asm.c
	gcc -g -m32 -o inline_asm inline_asm.c

mainprog: mainprog.c funcs_for_C.s
	#as -gstabs --32 -o funcs_for_C.o funcs_for_C.s
	#gcc -m32 -g -o funcs_for_C funcs_for_C.o
	gcc -m32 -g -o mainprog mainprog.c funcs_for_C.s

clean:
	rm -rf $(TARGETS)
