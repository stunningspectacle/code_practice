all: hello

hello: hello.c hello.lds
	gcc -c hello.c 
	ld -o hello -T hello.lds hello.o
	objdump -D hello > hello.obj


