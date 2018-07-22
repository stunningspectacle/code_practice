#!/bin/bash
#echo "gcc -c hello.s"
echo "as --32 -o hello.o hello.s"
as --32 -o hello.o hello.s
#gcc -c hello.s

#echo "ld -melf_x86_64 -o hello hello.o"
echo "ld -melf_i386 -o hello hello.o"
ld -melf_i386 -o hello hello.o

#echo "sstrip hello"
#sstrip hello

echo "strip hello"
strip hello

echo "wc -c hello"
wc -c hello

echo "./hello"
./hello
