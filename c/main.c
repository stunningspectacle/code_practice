#include "a.h"

#define min(a, b) ({ \
    const typeof(a) _a = (a); \
    const typeof(b) _b = (b); \
    (void) (&_a == &_b); \
    _a < _b ? _a : _b; })
#define pr_x(fmt, arg...) printf("xxxxx" fmt, ##arg)

struct num_t {
    int a;
    int b;
    int c;
    char abc;
};

int main(void)
{
	int a = 100;
	int b = 101;
	struct num_t mynum = {
		.a = 100,
		.b = 200,
		.c = 300,
	};
	struct num_t *np = &mynum;
	pr_x("%d\n", min(a, b));
	pr_x("adsfasdf\n");
	pr_x("%d, %d, %d\n", mynum.a, mynum.b, mynum.c);
	printf("sizeof(unsigned short)=%d\n", sizeof(unsigned short));
	printf("sizeof(np->abc)=%d\n", sizeof(np->abc));
	printf("sizeof(mynum.abc)=%d\n", sizeof(mynum.abc));
	return 0;
}
