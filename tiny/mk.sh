#!/bin/bash
#echo "gcc -c hello.s"
echo "cat hello.s:"
cat hello.s

echo "as --32 -o hello.o hello.s"
as --32 -o hello.o hello.s
#gcc -c hello.s

#echo "ld -melf_x86_64 -o hello hello.o"
echo "ld -melf_i386 -o hello hello.o"
ld -melf_i386 -o hello hello.o

#echo "strip hello"
#strip hello

echo "sstrip hello"
sstrip hello
#
#echo "wc -c hello"
#wc -c hello
#
#hexdump -C -n 52 hello
#echo
#hexdump -C -s 52 -n 32 hello
#echo
#hexdump -C -s 84 hello

#echo "mv hello hello\ World!"
#mv hello hello\ World!

#echo "./hello World!"
#./hello\ World!
