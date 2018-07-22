extern int shared;

int global_noinit;
int main() {
	int a = 10;
	swap(&a, &shared);
}
