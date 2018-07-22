#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef DEBUG
#define PRINTF(fmt, args...) printf(fmt, ##args)
#else
#define PRINTF(fmt, args...)
#endif

enum State {
	EMPTY = 0,
	LEGITIMATE,
	DELETED,
};

struct hashEntry {
	unsigned int value;
	enum State state;
	int repeat;
};

struct hashTable {
	int size;
	int filled;
	struct hashEntry *entries;
};

struct hashTable *insert(struct hashTable *t, unsigned int value, int repeat);
void dump(struct hashTable *t);

struct hashTable *table;
int collision;

int isPrime(int n)
{
	int i;

	for (i = 2; i * i <= n; i++) {
		if (n % i == 0)
			return 0;
	}

	return 1;
}

int nextPrime(int size) {
	int i;
	int tmp = size;

	for (i = size; !isPrime(i); i++)
		;
	printf("get prime %d\n", i);
	return i;
}

struct hashTable *createHashTable(int size)
{
	int realSize;
	struct hashTable *t;

	realSize = nextPrime(size);
	t = malloc(sizeof(struct hashEntry) * realSize +
			sizeof(struct hashTable));
	if (t == NULL) {
		printf("%s: Out of memroy!\n", __func__);
		return NULL;
	}
	memset(t, 0, sizeof(struct hashEntry) * realSize +
			sizeof(struct hashTable));
	t->size = realSize;
	t->entries = (struct hashEntry *)(t + 1);

	return t;
}

void freeHashTable(struct hashTable *t)
{
	if (t != NULL)
		free(t);
}

int hash(unsigned int value, int tableSize)
{
	int tmp = value;
	unsigned int hashValue = 0;

	while (value) {
		hashValue = (hashValue << 5)  + (value % 10);
		value /= 10;
	}

	PRINTF("%d, hashValue=%d, tsize=%d, hash to %d\n",
			tmp, hashValue, tableSize, hashValue % tableSize);

	return hashValue % tableSize;
}

int find(struct hashTable *t, unsigned int value)
{
	int pos;
	int collisionNum;
	struct hashEntry *entry;
	
	pos = hash(value, t->size);
	collisionNum = 0;
	while (t->entries[pos].state != EMPTY &&
			t->entries[pos].value != value) {
		collision++;
		pos += 2 * ++collisionNum - 1;
		if (pos >= t->size)
			pos -= t->size;
	}
	return pos;
}

struct hashTable *rehash(struct hashTable *old)
{
	int i;
	struct hashTable *newTable;

	printf("rehashing table ...\n");
	printf("old table:\n");
	dump(old);

	newTable = createHashTable(old->size * 2);
	if (newTable == NULL)
		return NULL;
	for (i = 0; i < old->size; i++) {
		if (old->entries[i].state == LEGITIMATE) {
			insert(newTable,
				old->entries[i].value, old->entries[i].repeat);
		}
	}
	freeHashTable(old);
	printf("new table:\n");
	dump(newTable);

	return newTable;
}

struct hashTable *insert(struct hashTable *t, unsigned int value, int repeat)
{
	int pos;
	struct hashTable *newTable;

	pos = find(t, value);
	if (t->entries[pos].state == LEGITIMATE) {
		t->entries[pos].repeat += repeat;
	} else if (t->entries[pos].state == EMPTY) {
		t->entries[pos].state = LEGITIMATE;
		t->entries[pos].value = value;
		t->entries[pos].repeat += repeat;
		t->filled++;

		if (t->filled >= (t->size >> 1)) {
			newTable = rehash(t);
			return newTable;
		}
	}

	return t;
}

void dump(struct hashTable *t)
{
	int i;
	int count = 0;

	for (i = 0; i < t->size; i++) {
		if (t->entries[i].state == LEGITIMATE) {
			count += t->entries[i].repeat;
			printf("%4d", t->entries[i].value);
		} else {
			printf("%4s", "---");
		}
		if (i > 0 && i % 30 == 0)
			printf("\n");
	}
	printf("\n");
	printf("%d values in total, collison:%d times\n",
			count, collision);
}

int main(int argc, char *argv[])
{
	char buf[16];
	unsigned int value;

	if (argc != 2) {
		printf("usage: %s: file\n", argv[0]);
		return 0;
	}

	FILE *file = fopen(argv[1], "r+");
	if (!file) {
		perror("Open file failed");
		return 0;
	}

	table = createHashTable(50);
	while (fgets(buf, sizeof(buf), file)) {
		value = atoi(buf);
		table = insert(table, value, 1);
	}

	printf("###############################final table############################\n");
	dump(table);
		
}
