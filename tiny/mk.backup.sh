#!/bin/bash
echo "gcc -c hello.s"
gcc -c hello.s

echo "ld --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o hello -L/usr/lib32/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.6 -L/usr/lib/gcc/x86_64-linux-gnu/4.6/../../../x86_64-linux-gnu -L/usr/lib/gcc/x86_64-linux-gnu/4.6/../../../../lib -L/lib/x86_64-linux-gnu -L/lib/../lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/../lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib32 -L/usr/lib/gcc/x86_64-linux-gnu/4.6/../../.. hello.o  -lc" 
ld --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o hello -L/usr/lib32/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.6 -L/usr/lib/gcc/x86_64-linux-gnu/4.6/../../../x86_64-linux-gnu -L/usr/lib/gcc/x86_64-linux-gnu/4.6/../../../../lib -L/lib/x86_64-linux-gnu -L/lib/../lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/../lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib32 -L/usr/lib/gcc/x86_64-linux-gnu/4.6/../../.. hello.o  -lc 

echo "strip hello"
strip hello

echo "strip -R .hash -R .gnu.version -R .eh_frame -R .eh_frame_hdr hello"
strip -R .hash -R .gnu.version -R .eh_frame -R .eh_frame_hdr hello

echo "wc -c hello"
wc -c hello

echo "./hello"
./hello
