#ifndef _MYSTACK_H
#define _MYSTACK_H

#include <string.h>
#define DEFINE_STACK(type, name)                        \
struct stack_##type {                                   \ 
	type elements[50];                              \
	int top;                                        \
        void (*push)(struct stack_##type *, type);      \
        type (*pop)(struct stack_##type *);             \
        type (*peek)(struct stack_##type *);            \
        int (*isEmpty)(struct stack_##type *);          \
        void (*dump)();                                 \
};                                                      \
struct stack_##type name;                               \
\
void push_##type(struct stack_##type *stack, type element) {\
	stack->elements[stack->top++] = element;\
}\
\
type pop_##type(struct stack_##type *stack) {\
	return stack->elements[--stack->top];\
}\
\
type peek_##type(struct stack_##type *stack) {\
	return stack->elements[stack->top - 1];\
}\
\
int isEmpty_##type(struct stack_##type *stack) {\
	if (stack->top == 0)\
		return 1;\
	return 0;\
}\
\
void dump_stack_##type(struct stack_##type *stack) {    \
    int i;                  \
    \
    for (i = 0; i < stack->top; i++) { \
        if (sizeof(type) == 1) {    \
            printf("%c ", stack->elements[i]); \
        } else { \
            printf("%d ", stack->elements[i]); \
        } \
    } \
    printf("\n");\
}

#define init_stack(type, name)                           \
do {                                                    \
    memset(&name, 0, sizeof(struct stack_##type));      \
    name.push = push_##type;                             \
    name.pop = pop_##type;                               \
    name.peek = peek_##type;                             \
    name.isEmpty = isEmpty_##type;                       \
    name.dump = dump_stack_##type;                       \
} while (0)

#endif
