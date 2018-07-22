cal: cal.c parse.c
	gcc -o cal cal.c parse.c
list: list.c
	gcc -o list list.c
tree: tree.c
	gcc -o tree tree.c
hash: hashTable.c
	gcc -o hash hashTable.c
try: try.c
	gcc -o try try.c
heap: heap.c
	gcc -o heap heap.c
sort: sort.c
	gcc -o sort sort.c
tinyHello: tinyHello.lds print.c
	gcc -c -fno-builtin print.c
	ld -e nomain -T tinyHello.lds -o tinyHello print.o
subString: largestSubString.c
	gcc -o subString largestSubString.c

