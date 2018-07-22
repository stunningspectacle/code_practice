#include "stack.h"
#include "cal.h"

DEFINE_STACK(char, stack)

void parseBuf(char *buf, struct ops *head) {
	char *p;
	char num_buf[16];
	char num_buf_index = 0;
	char topChar;

	init_stack(char, stack);
	p = buf;
	for (p = buf; *p; p++) {
		if (isdigit(*p)) {
			num_buf[num_buf_index++] = *p;
			continue;
		} else if (num_buf_index != 0) {
			num_buf[num_buf_index] = '\0';
			insertNum(atoi(num_buf), head);
			num_buf_index = 0;
		}
		if (!isSymbol(*p))
			continue;

		if (stack.isEmpty(&stack)) {
			stack.push(&stack, *p);
			continue;
		}

		topChar = stack.peek(&stack);
		switch (*p) {
		case '*':
			while (topChar == '*') {
				insertSymbol(stack.pop(&stack), head);
				topChar = stack.peek(&stack);
			}
			stack.push(&stack, *p);
			break;
		case '+':
			while (topChar == '+' || topChar == '*') {
				insertSymbol(stack.pop(&stack), head);
				topChar = stack.peek(&stack);
			}
			stack.push(&stack, *p);
			break;
		case '(':
			stack.push(&stack, *p);
			break;
		case ')':
			while (topChar != '(') {
				insertSymbol(stack.pop(&stack), head);
				topChar = stack.peek(&stack);
			}
			stack.pop(&stack);
			break;
		default:
			printf("Error! Should not be here!\n");
		}
	}
	printf("\n");
	printf("last symbol:\n");
	if (num_buf_index != 0) {
		num_buf[num_buf_index] = '\0';
		insertNum(atoi(num_buf), head);
	}
	while (!stack.isEmpty(&stack))
		insertSymbol(stack.pop(&stack), head);
}
#undef ElementType
