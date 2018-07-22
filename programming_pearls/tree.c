#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define DEBUG 0


struct biTree {
	int num;
	struct biTree *left;
	struct biTree *right;
};

struct tree {
	int num;
	struct tree *sibling;
	struct tree *firstChild;
};

int count;
struct biTree *insert(struct biTree *parent, int num) {
	if (parent == NULL) {
		++count;
		parent = malloc(sizeof(struct biTree));
		parent->num = num;
		parent->left = NULL;
		parent->right = NULL;
		return parent;
	}
	if (num < parent->num)
		parent->left = insert(parent->left, num);
	if (num > parent->num)
		parent->right = insert(parent->right, num);
	return parent;
}

struct biTree *find(struct biTree *parent, int num) {
	if (parent == NULL)
		return NULL;
	if (num < parent->num)
		return find(parent->left, num);
	if (num > parent->num)
		return find(parent->right, num);
	return parent;
}

struct biTree *findMin(struct biTree *T) {
	if (T == NULL)
		return NULL;
	if (T->left == NULL)
		return T;
	return findMin(T->left);
}

struct biTree *findMax(struct biTree *T) {
	if (T == NULL)
		return NULL;
	if (T->right == NULL)
		return T;
	return findMax(T->right);
}

void doDump(struct biTree *parent) {
	if (parent == NULL)
		return;
	doDump(parent->left);
	printf("%d ", parent->num);
	doDump(parent->right);
}

void dump(struct biTree *parent) {
	doDump(parent);
	printf("\n");
	printf("count=%d\n", count);
}

void treeDir(const char *path, int indent) {
	int i;
	DIR *dirp;
	struct dirent *entry;
	char pathBuf[128];
	char *type;
	const char *src;
	char *dest, *p;

	if ((dirp = opendir(path)) == NULL) {
		perror(path);
		return;
	}

	src = path;
	dest = pathBuf;
	while (*src) {
		*dest++ = *src++;
	}
	if (DEBUG){
		*dest = '\0';
		printf("pathBuf: %s\n", pathBuf);
	}

	while ((entry = readdir(dirp)) != NULL) {
		if (!strcmp(entry->d_name, ".") ||
			!strcmp(entry->d_name, "..")) {
			continue;
		}
		p = dest;

		if (entry->d_type == DT_BLK)
			type = "DT_BLK";
		else if (entry->d_type == DT_CHR)
			type = "DT_CHR";
		else if (entry->d_type == DT_FIFO)
			type = "DT_FIFO";
		else if (entry->d_type == DT_LNK)
			type = "DT_LNK";
		else if (entry->d_type == DT_REG)
			type = "DT_REG";
		else if (entry->d_type == DT_SOCK)
			type = "DT_SOCK";
		else if (entry->d_type == DT_DIR)
			type = "DT_DIR";
		else 
			type = "UNKNOW";


		for (i = 0; i < indent; i++)
			printf("\t");
		printf("%s\n", entry->d_name);
		if (entry->d_type == DT_DIR) {
			if (*(p - 1) != '/')
				*p++ = '/';
			src = entry->d_name;
			while (*src) {
				*p++ = *src++;
			}
			*p = '\0';
			treeDir(pathBuf, indent + 1);
		}
	}
}

void main(int argc, char *argv[]) {
	char buf[32];
	FILE *file;
	struct biTree *root = NULL;
	struct biTree *p;
	int num;

	if (argc != 2) {
		printf("Usage %s dirname\n", argv[0]);
		return;
	}
	if ((file = fopen(argv[1], "r")) == NULL) {
		perror(argv[1]);
		return;
	}

	while (fgets(buf, sizeof(buf), file)) {
		num = atoi(buf);
		p = insert(root, num);
		if (root == NULL)
			root = p;
	}
	dump(root);
	printf("min=%d, max=%d\n", findMin(root)->num, findMax(root)->num);
	/*
	while (gets(buf)) {
		num = atoi(buf);
		p = find(root, num);
		if (p == NULL)
			printf("No %d\n", num);
		else 
			printf("Yes %d\n", num);
	}
	*/
}
