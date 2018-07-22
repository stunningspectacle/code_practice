#include <stdio.h>
#include <stdint.h>

int myadd()
{
	int i;
	int j = 0;
	unsigned long sum = 0;

	for (i = 0; i < 10000; i++, j++)
		sum *= (i + j);
}

int func(int a, int b, int c, int d, int e)
{
	int i = 0;
	int j = 1;

	for (i = 0; i < 2000; i++) {
		a += i * j;
		myadd();
	}

	return a + b + c + d + e;
}

#define UINT32 unsigned int 
typedef union _IPC_HEADER
{
	UINT32 value;
	struct
	{
		UINT32 length       :10;    //[0:9]
		UINT32 protocol     :4;     //[10:13]
		UINT32 reserved1    :2;     //[14:15]
		UINT32 command      :4;     //[16:19]
		UINT32 reserved2    :9;     //[20:28]
		UINT32 options      :2;     //[29:30]
		UINT32 busy         :1;     //[31]
	};

}IPC_HEADER;

int test_as(int a)
{
	int *ok;
	int b = 100;

	ok = &a;
	return *ok + b;
}

int args(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15)
{
	int j = a1 + a2 + a3 + a4 + a5 + a6;
	int i = a7 + a8 + a9 + a10 + a11 + a12 + a13;

	return i * j;
}

int div_zero(int d0, int d1) {
	int i;
	int j;
	int k;

	i = d0;
	j = d1;
	k = d0 / d1;

	return k;
}

void main(void) {
	int n0, n1, i;

	IPC_HEADER expectedIPC;
	expectedIPC.value = 0;
	printf("value=0x%x\n", expectedIPC.value);

	expectedIPC.protocol = 0xD;
	expectedIPC.command = 1;
	expectedIPC.busy = 1;

	printf("value%n=0x%x%n\n", &n0, expectedIPC.value, &n1);
	printf("%d %d chars\n", n0, n1);

	func(1, 2, 3, 4, 5);

	div_zero(10, 0);
}
