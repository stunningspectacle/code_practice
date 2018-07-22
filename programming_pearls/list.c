#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct node {
	int num;
	struct node *prev;
	struct node *next;
};

void printLots(struct node *L, struct node *P) {
	int n;
	struct node *index = P;
	struct node *shower;

	while (index != NULL) {
		shower = L;
		n = index->num;
		while (n && shower) {
			shower = shower->next;
			n--;
		}
		if (n != 0) {
			printf("%d is beyond the capacity of shower\n", n);
		} else {
			printf("Value of %d is %d\n", n, shower->num);
		}
	}
}

void swapList(struct node *head, int num) {
	struct node *p = head;
	struct node *target;
	struct node *next;

	while (num != 1&& p) {
		p = p->next;
		num--;
	}

	target = p->next;
	next = target->next;

	target->next = next->next;
	next->next = target;
	p->next = next;
}

void dumpList(struct node *head) {
	struct node *p = head->next;

	while (p) {
		printf("%d ", p->num);
		p = p->next;
	}
	printf("\n");
}

void swapBiList(struct node *head, int num) {
	struct node *p = head->next;
	struct node *next;

	while (num && p) {
		p = p->next;
		num--;
	}
	next = p->next;

	p->prev->next = next;
	next->prev = p->prev;
	p->next = next->next;
	next->next->prev = p;
	next->next = p;
	p->prev = next;
}

void dumpBiList(struct node *head) {
	struct node *p = head->next;

	while (p && p->next) {
		printf("%d ", p->num);
		p = p->next;
	}
	printf("%d ", p->num);
	printf("\n");

	while (p->prev != p) {
		printf("%d ", p->num);
		p = p->prev;
	}
	printf("\n");
}

void main(void) {
	int i;
	struct node head;
	struct node *tail;
	struct node *p;


	head.prev = head.next = &head;
	tail = &head;
	for (i = 0; i < 10; i++) {
		p = malloc(sizeof(struct node));
		p->num = i;
		p->next = NULL;
		p->prev = tail;
		tail->next = p;
		tail = p;
	}
	dumpBiList(&head);
	swapBiList(&head, 5);
	dumpBiList(&head);
}
