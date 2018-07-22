#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "stack.h"

struct mystack *createStack() {
	struct mystack *stack;

	stack = malloc(sizeof(struct mystack));
	memset(stack, 0, sizeof(struct mystack));

	return stack;
}

void push(struct mystack *stack, void *element, int size) {
	void *p;
	
	p = malloc(size);
	memcpy(p, element, size);
	stack->elements[stack->top++] = num;
}

int pop(struct mystack *stack) {
	return stack->elements[--stack->top];
}

int peek(struct mystack *stack) {
	printf("peek: %c\n", stack->elements[stack->top - 1]);
	return stack->elements[stack->top - 1];
}

int isEmpty(struct mystack *stack) {
	if (stack->top == 0)
		return 1;
	return 0;
}

void dumpStack(struct mystack* stack) {
	int i;

	for (i = 0; i < stack->top; i++) {
		printf("%d ", stack->elements[i]);
	}
	printf("\n");
}

void testStack(void) {
	struct mystack *stack;
	int i;

	stack = createStack();

	for (i = 0; i < 10; i++) {
		push(stack, i);
	}
	dump_stack(stack);
	pop(stack);	
	pop(stack);	
	dump_stack(stack);
	push(stack, 100);
	dump_stack(stack);
}
