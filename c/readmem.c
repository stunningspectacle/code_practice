#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define READSIZE 256
#define PCI_CONFIG_START 0xe0000000

int main(char argc, char *argv[])
{
	int i, fd;
	int bus, device, function;
	unsigned long addr;
	u_int32_t *vm;
	u_int32_t *tmp;

	if (argc == 2)
		addr = strtol(argv[1], NULL, 16);
	else if (argc == 4) {
		bus = atoi(argv[1]);
		device = atoi(argv[2]);
		function = atoi(argv[3]);
		addr = PCI_CONFIG_START +
			(bus << 20) + (device << 15) + (function << 12);
	} else {
		printf("usage1. %s bus device function\n", argv[0]);
		printf("usage2. %s address\n", argv[0]);
		return EINVAL;
	}
	printf("addr = 0x%lx\n", addr);

	fd = open("/dev/mem", O_RDONLY);	
	if (fd < 0) {
		perror(strerror(errno));
		return -EFAULT;
	}
	vm = mmap(NULL, READSIZE, PROT_READ, MAP_SHARED, fd, addr);
	if (vm == MAP_FAILED) {
		perror(strerror(errno));
		return -EFAULT;
	}
	printf("mmap @%p\n", vm);
	for (i = 0; i < READSIZE; i += 8) {
		tmp = vm + i;
		printf("%x %x %x %x %x %x %x %x\n",
				tmp[0], tmp[1], tmp[2], tmp[3],
				tmp[4], tmp[5], tmp[6], tmp[7]);
	}

	return 0;
}
