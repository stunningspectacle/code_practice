#include <stdio.h>
#include <stdlib.h>

#include "cal.h"
#include "stack.h"

char symbolList[] = {'+', '*', '(', ')'};

char *buf = "10*20+30*40+(60*5+30*20)+80";
char newBuf[128];

struct ops {
	char isNumber;
	union {
		char op;
		int num;
	};
	struct ops *next;
};

struct ops chain_head;

void dumpChain() {
	struct ops *p;

	printf("Dump chain:\n");
	p = chain_head.next;
	while (p != NULL) {
		if (p->isNumber != 0) {
			printf("%d ", p->num);
		} else {
			printf("%c ", p->op);
		}
		p = p->next;
	}
	printf("\n");
}

int isSymbol(char c) {
	int i;

	for (i = 0; i < sizeof(symbolList)/sizeof(symbolList[0]);
			i++) {
		if (c == symbolList[i])
			return 1;
	}
	return 0;
}

void insertSymbol(char symbol, struct ops *head) {
	struct ops *op;
	struct ops *p;

	p = head;
	while (p->next)
		p = p->next;

	printf("Insert symbol: %c\n", symbol);
	op = malloc(sizeof(struct ops));
	op->isNumber = 0;
	op->op = symbol;
	op->next = NULL;
	p->next = op;
}

void insertNum(int num, struct ops *head) {
	struct ops *op;
	struct ops *p;

	p = head;
	while (p->next)
		p = p->next;
	printf("Insert num: %d\n", num);
	op = malloc(sizeof(struct ops));
	op->isNumber = 1;
	op->num = num;
	op->next = NULL;
	p->next = op;
}

DEFINE_STACK(int, stack)
void cal(void) {
	struct ops *op;
	int op1, op2;
	int res;

	init_stack(int, stack);

	for (op = chain_head.next; op != NULL; op = op->next) {
		if (op->isNumber) {
			stack.push(&stack, op->num);
			continue;
		} else {
			op1 = stack.pop(&stack);
			op2 = stack.pop(&stack);
			if (op->op == '+') {
				res = op1 + op2;
			} else if (op->op == '*') {
				res = op1 * op2;
			}
			stack.push(&stack, res);
			printf("op1 = %d, op2 = %d, res = %d\n",
					op1, op2, res);
		}
	}
	printf("res = %d\n", res);
}

void main(void) {
	//gets(buf);
	parseBuf(buf, &chain_head);
	printf("%s\n", buf);
	dumpChain();
	cal();
}
