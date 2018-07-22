#include <stdio.h>


int main(void)
{
	char buf[256];

	sprintf(buf, "count:%%d, chan:%%d, dst:0x%%x\n");

	printf(buf, 0, 2, 0x12345);
}
