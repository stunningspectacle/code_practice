struct A {
	int a;
	char b;
	int c;
	int d;
};

struct B {
	int a;
	char b;
	int c;
};


int main(void)
{
	struct A Aa = { 0 };
	struct B Ba = { 0 };


	struct A *Ap = &Aa;
	struct B *Bp;

	Bp = (struct B*)Ap;
}
