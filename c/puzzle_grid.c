#include <stdio.h>
#include <stdlib.h>

/*
 * OOOO
 * OOOO
 * OOOO
 * OOOO
 */

struct element {
	unsigned int content;
};

struct element all_elements[] = {
	{
		/* O */
		.content = 0x1,
	},
	{
		/* OO */
		.content = 0x3,
	},
	{
		/* OOO */
		.content = 0x7,
	},
	{
		/*OOOO*/
		.content = 0xf,
	},
	{
	 	/*  O
		 * OOO
		 */
		.content = (0x7 << 4) | 0x2,
	},
	{
		/* OOO
		 *   O
		 */
		.content = (0x1 << 6) | 0x7,
	},
	{
		/* O
		 * OO
		 *  O
		 */
		.content = (0x1 << 9) | (0x3 << 4) | 0x1,
	}
};

unsigned int get_block(int *puzzle, int width, int x, int y)
{
	int i, *start;
	unsigned int block = 0;

	for (i = 0; i < 4; i++ ) {
		start = puzzle + width * (x + i);
		block = (block << 4) |
			(start[0]) |
			(start[1] << 1) |
			(start[2] << 2) |
			(start[3] << 3);
	}

	return block;
}


int array[5][5] = {{0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1},
		{2, 2, 2, 2, 2},
		{3, 3, 3, 3, 3},
		{4, 4, 4, 4, 4}};

void print_array(int a[5][5])
{
	printf("%d, %d, %d, %d\n", a[0][0], a[1][0], a[2][0], a[3][0]);
}

void main()
{
	print_array(array);
}


