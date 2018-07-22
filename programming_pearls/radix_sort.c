#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct node {
	int num;
	struct node *next;
	struct node *prev;
};

void dump(struct node **chain) {
	int i;
	struct node *tmp;
	
	printf("start dump:\n");
	for (i = 0; i < 10; i++) {
		tmp = chain[i];
		printf("radix %d: ", i);
		if (tmp == NULL) {
			printf("\n");
			continue;
		}
		do {
			printf("%d ", tmp->num);
			tmp = tmp->next;
		} while (tmp != chain[i]);
		printf("\n");
	}
	printf("------------dump end---------------------\n");
}

void addNum(struct node **head, int num) {
	struct node *newNode;

	newNode = malloc(sizeof(struct node));
	newNode->num = num;

	if (*head == NULL) {
		*head = newNode;
		newNode->next = newNode->prev = newNode;
	} else {
		newNode->next = *head;
		(*head)->prev->next = newNode;
		newNode->prev = (*head)->prev;
		(*head)->prev = newNode;
	}
}

struct node **sort(struct node **chain, int power) {
	int i;
	int base = 1;
	int radix;
	struct node **barrel;
	struct node *old;
	struct node *tmp;

	barrel = calloc(10, sizeof(struct node *));
	//memset(barrel, 0, sizeof(struct node *) * 10);

	while (--power)
		base *= 10;

	for (i = 0; i < 10; i++) {
		old = chain[i];
		if (old == NULL)
			continue;
		while (1) {
			radix = (old->num / base) % 10;
			//printf("num=%d, radix=%d\n", old->num, radix);
			addNum(&barrel[radix], old->num);
			//dump(barrel);
			if (old == old->next) {
				free(old);
				break;
			}
			tmp = old;
			old->prev->next = old->next;
			old->next->prev = old->prev;
			old = old->next;
			free(tmp);
		} 
		chain[i] = NULL;
		//printf("barrel %d, chain:\n", i);
		//dump(chain);
	}
	free(chain);

	return barrel;
}

void main(int argc, char **argv) {
	int num;
	int remainder;
	FILE *file;
	char buf[16];
	struct node **foo;
	int max;
	int radix;

	if (argc != 3) {
		printf("Usage: %s file max\n", argv[0]);
		return;
	}

	if ((file = fopen(argv[1], "r")) == NULL) {
		perror(NULL);
		return;
	}
	
	if ((max = atoi(argv[2])) < 0) {
		printf("max value error!\n");
		return;
	}

	foo = calloc(10, sizeof(struct node *));
	//memset(foo, 0, sizeof(struct node *) * 10);

	while (fgets(&buf, sizeof(buf), file) != NULL) {
		num = atoi(buf);
		//printf("%d\n", num);
		remainder = num % 10;
		addNum(&foo[remainder], num);
	}

	radix = 1;
	while (max > 0) {
		foo = sort(foo, radix);
		dump(foo);
		max /= 10;
		radix++;
	}
}
