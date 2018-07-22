#!/bin/bash
function cmd() {
    echo "$@"
    $@
}

#cmd hexdump -C -n 52 $1
#cmd hexdump -C -s 52 -n 32 $1
#cmd hexdump -C -s 84 -n 16 $1

cmd dd if=hello of=hello bs=1 skip=84 seek=4 count=10 conv=notrunc
echo "echo -ne \"\xeb\x10\" | dd of=hello bs=1 count=2 seek=14 conv=notrunc"
echo -ne "\xeb\x10" | dd of=hello bs=1 count=2 seek=14 conv=notrunc
cmd dd if=hello of=hello bs=1 skip=94 seek=32 count=6 conv=notrunc
cmd dd if=hello of=hello bs=1 count=1 skip=84 seek=84

echo "echo -ne \"\x04\" | dd of=hello bs=1 count=1 seek=24 conv=notrunc"
echo -ne "\x04" | dd of=hello bs=1 count=1 seek=24 conv=notrunc


cmd hexdump -C -n 52 $1
cmd hexdump -C -s 52 -n 32 $1
