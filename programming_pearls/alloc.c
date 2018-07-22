#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define END ((unsigned short)(~0))

void *pool;
unsigned short *cursor;
unsigned short avail;

unsigned int max_member;
unsigned int member_size;

void init(int nmumb, int size) {
	int i;

	pool = calloc(nmumb, size);
	max_member = nmumb;
	member_size = size;
	cursor = calloc(nmumb, sizeof(unsigned short));
	for (i = 0; i < nmumb; i++)
		cursor[i] = i + 1;
	cursor[nmumb - 1] = END;
	avail = 0;
}

void dump_cursor() {
	unsigned short tmp;

	tmp = avail;
	printf("avail=%d:\n", avail); 
	while (tmp != END) {
		printf("%d->%d\n", tmp, cursor[tmp]);
		tmp = cursor[tmp];
	}
}
	
void *myalloc() {
	void *ret;

	if (avail == END) {
		printf("alloc failed\n");
		return;
	}
	ret = pool + avail * member_size;
	avail = cursor[avail];
	return ret;
}

void myfree(void *item) {
	unsigned short pos;

	pos = (item - pool) / member_size; 
	printf("pool=%p, item=%p, pos=%d\n", pool, item, pos);
	cursor[pos] = avail;
	avail = pos;
}

struct node {
	int num;
	struct node *prev;
	struct node *next;
};

void main(void) {
	int i;
	struct node *ss[5];

	init(10, sizeof(struct node));

	for (i = 0; i < sizeof(ss)/sizeof(ss[0]); i++) {
		ss[i] = myalloc();
	}
	dump_cursor();
	myfree(ss[3]);
	dump_cursor();
	myfree(ss[1]);
	dump_cursor();
}
