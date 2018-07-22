#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SENTINEL (-1)
#define DEBUG

#ifdef DEBUG
#define PRINTF(fmt, args...) printf(fmt, ##args)
#else
#define PRINTF(fmt, args...) 
#endif

struct heap {
	int capacity;
	int size;
	int elements[0];
};

#define SORT_SIZE 100

int sortBuf[SORT_SIZE];

struct heap *createHeap(int capacity) {
	struct heap *p;

	p = malloc(sizeof(struct heap) + (capacity + 1) * sizeof(int));
	if (p == NULL) {
		perror("Creat heap failed");
		return NULL;
	}
	p->capacity = capacity;
	p->size = 0;
	p->elements[0] = SENTINEL;

	PRINTF("Create heap with capacity %d\n", capacity);

	return p;
}

struct heap * insert(struct heap *h, int num) {
	int pos;

	if (h->size == h->capacity) {
		printf("heap is full, insert failed\n");
		return NULL;
	}

	for (pos = ++h->size; num < h->elements[pos / 2]; pos /= 2)
		h->elements[pos] = h->elements[pos / 2];
	h->elements[pos] = num;

	return h;
}

int checkHeap(struct heap *h, int pos) {
	if (pos * 2 > h->size) {
		return 0;
	}
	if (h->elements[pos * 2] < h->elements[pos]) {
		printf("elements[%d]=%d bigger than elements[%d]=%d, heap error!\n",
				pos, h->elements[pos], 2 * pos, h->elements[2 * pos]);
		return 1;
	}
	if (pos * 2 + 1 <= h->size && h->elements[pos * 2 + 1] < h->elements[pos]) {
		printf("elements[%d]=%d bigger than elements[%d]=%d, heap error!\n",
				pos, h->elements[pos], 2 * pos + 1, h->elements[2 * pos + 1]);
		return 1;
	}
	return checkHeap(h, 2 * pos) + checkHeap(h, 2 * pos + 1);
}

int delMin(struct heap *h) {
	int pos, min;
	
	if (h->size == 0) {
		printf("heap is empty, delMin failed\n");
		return -1;
	}
	min = h->elements[1];

	for (pos = 2; pos < h->size; pos *= 2) {
		if (pos + 1 < h->size && h->elements[pos + 1] < h->elements[pos]) {
			pos += 1;
		}
		if (h->elements[pos] < h->elements[h->size]) {
			h->elements[pos / 2] = h->elements[pos];
		} else
			break;
	}
	h->elements[pos / 2] = h->elements[h->size--];

	return min;
}

int delMin2(struct heap *h) {
	int i, min, child;
	
	if (h->size == 0) {
		printf("heap is empty, delMin failed\n");
		return -1;
	}
	min = h->elements[1];

	for (i = 1; i * 2 < h->size; i = child) {
		child = i * 2;
		if (child + 1 < h->size && h->elements[child + 1] < h->elements[child])
			child++;

		if (h->elements[child] < h->elements[h->size]) {
			h->elements[i] = h->elements[child];
		} else
			break;
	}
	h->elements[i] = h->elements[h->size--];

	return min;
}

void dump(struct heap *h) {
	int i;

	for (i = 1; i <= h->size; i++)
		printf("%d ", h->elements[i]);
	printf("\n-----------------------dump completed------------------\n");
}

void percolateDown(struct heap *h, int pos) {
	int i, child, target;

	target = h->elements[pos];
	for (i = pos; 2 * i <= h->size; i = child) {
		child = 2 * i;
		if (child < h->size && h->elements[child + 1] < h->elements[child])
			child++;
		if (h->elements[child] < target)
			h->elements[i] = h->elements[child];
		else
			break;
	}
	h->elements[i] = target;
}

void buildHeap(struct heap *h) {
	int i;

	for (i = h->size / 2; i > 0; i--) {
		percolateDown(h, i);
	}
}

int delMin3(struct heap *h)
{
	int min;

	if (h->size == 0) {
		printf("heap is empty, delMin failed\n");
		return -1;
	}

	min = h->elements[1];
	h->elements[1] = h->elements[h->size--];
	percolateDown(h, 1);

	return min;
}

void main(int argc, char *argv[]) {
	struct heap *h;
	FILE *file;
	char buf[8];
	int old = -1, num;

	if (argc != 3) {
		printf("Usage: %s file count\n", argv[0]);
		return;
	}

	file = fopen(argv[1], "r");
	if (!file) {
		perror("open file failed");
		return;
	}

	num = atoi(argv[2]);
	h = createHeap(num);
	while (fgets(buf, sizeof(buf), file)) {
		num = atoi(buf);
		/*
		if (insert(h, num) == NULL)
			break;
		*/
		h->elements[++h->size] = num;
	}

	buildHeap(h);

	if (checkHeap(h, 1) == 0) {
		printf("heap correct\n");
	} else {
		printf("heap incorrect\n");
		return;
	}
	//dump(h);

	while (h->size > 0) {
		//num = delMin(h);
		//num = delMin2(h);
		num = delMin3(h);
		printf("%d ", num);
		if (num < old) {
			printf("(----------------error, %d < %d---------------) ", num, old);
		}
		old = num;
	}
	printf("\n");
}
