#include <linux/pci_regs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

//#define DEBUG

#ifdef DEBUG
#define debug_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug_printf(fmt, ...)
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define SYS_PCI_PATH "/sys/bus/pci/devices/"

static const struct {
	char *name;
	u_int32_t devid;
} ish_dev_ids[] = {
	{ "SPT", 0xA135 },
	{ "SPT", 0x9D35 },
	{ "BXT", 0x0AA2 },
	{ "BXT", 0x5AA2 },
	{ "BXT", 0x1AA2 },
	{ "CNL", 0x9DFC },
	{ "ICL", 0x34FC },
	{ "CHV", 0x22D8 },
	{ "LKF", 0x2574 },
	{ "LKF", 0x98FC },
	{ "TGL", 0xA0FC },
	{ "EHL", 0x4B30 },
	{ "TEST", 0x100e },
};

static void print_lines(void *mem, size_t size)
{
	int i, j, n, remainder;
	u_int8_t *line;
	char *buf;
	char line_buf[256] = { 0 };

	for (i = 0; i < size / 16; i++) {
		line = mem + i * 16;
		n = 0;
		buf = line_buf;
		for (j = 0; j < 16; j++) {
			n = sprintf(buf, "%-3x ", line[j]);
			buf += n;
		}
		*buf = 0;
		printf("%-5x: %s\n", i * 16, line_buf);
	}

	remainder = size % 16;
	if (remainder == 0)
		return;

	line = mem + i * 16;
	n = 0;
	buf = line_buf;
	for (j = 0; j < remainder; j++) {
		n = sprintf(buf, "%-3x ", line[j]);
		buf += n;
	}
	*buf = 0;
	printf("%-5x: %s\n", i * 16, line_buf);
}

static void dump_phy_mem(unsigned long mem_start, size_t size)
{
	int n, fd;
	void *mem;

	if ((fd = open("/dev/mem", O_RDONLY)) < 0) {
		perror("Failed to open file");
		return;
	}
	
	mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, mem_start);
	if (mem == MAP_FAILED) {
		perror("mmap failed");
		return;
	}

	print_lines(mem, size);
}

static int dump_dev(char *dir_path)
{
	char path_buf[PATH_MAX];
	char *path;
	int i, n, config_fd;
	u_int32_t vendor_devid;
	int is_ish = 0;
	u_int32_t bar0 = 0, bar1 = 0;
	u_int8_t config_space[256];

	memcpy(path_buf, dir_path, strlen(dir_path));
	path = path_buf + strlen(dir_path);
	strcpy(path, "/config");
	debug_printf("try %s\n", path_buf);

	if ((config_fd = open(path_buf, O_RDONLY)) < 0) {
		printf("Failed to open %s\n", path_buf);
		return 0;
	}
	if ((n = read(config_fd, config_space, sizeof(config_space))) < 0) {
		printf("Failed to read %s\n", path_buf);
		close(config_fd);
		return 0;
	}
	close(config_fd);

	debug_printf("Got %d bytes form config space\n", n);

	vendor_devid = *(u_int32_t *)config_space;
	if ((vendor_devid & 0xffff) != 0x8086)
		return 0;

	for (i = 0; i < ARRAY_SIZE(ish_dev_ids); i++) {
		if (ish_dev_ids[i].devid == (vendor_devid >> 16)) {
			printf("Found ISH device on %s, devid:0x%x\n",
					ish_dev_ids[i].name,
					ish_dev_ids[i].devid);
			is_ish = 1;
		}
	}

	if (is_ish) {
		printf("pci config space:\n");
		print_lines(config_space, n);

		bar0 = *(u_int32_t *)(config_space + PCI_BASE_ADDRESS_0);
		if ((bar0 == 0) || (bar0 & PCI_BASE_ADDRESS_SPACE_IO)) {
			printf("Invalid bar0: 0x%x", bar0);
			return 0;
		}
		if (bar0 & PCI_BASE_ADDRESS_MEM_TYPE_64)
			bar1 = *(u_int32_t *)(config_space + PCI_BASE_ADDRESS_1);

		printf("\nbar0:\n");
		dump_phy_mem(((unsigned long)bar1 << 32) |
				(bar0 & PCI_BASE_ADDRESS_MEM_MASK), 256);
	}

	return is_ish;
}

int main(int argc, char *argv[])
{
	DIR *pci_dir;
	struct dirent *dir_ent;
	char path_buf[PATH_MAX] = SYS_PCI_PATH;
	char real_path_buf[PATH_MAX];
	char *path;
	int ish_found = 0;

	pci_dir = opendir(SYS_PCI_PATH);
	if (!pci_dir) {
		perror("Failed to open " SYS_PCI_PATH);
		return -1;
	}

	while (dir_ent = readdir(pci_dir)) {
		if (!strcmp(dir_ent->d_name, ".") || !strcmp(dir_ent->d_name, ".."))
			continue;
		path = path_buf + strlen(SYS_PCI_PATH);
		debug_printf("entry: name(%s), d_type(%u), d_reclen(%u), d_off(%lu), d_ino(%lu)\n",
			dir_ent->d_name, dir_ent->d_type,
			dir_ent->d_reclen, dir_ent->d_off, dir_ent->d_ino);
		memcpy(path, dir_ent->d_name, strlen(dir_ent->d_name));
		path += strlen(dir_ent->d_name);
		*path++ = '/';
		*path = 0;
		debug_printf("%s\n", path_buf);
		path = realpath(path_buf, real_path_buf);
		debug_printf("%s\n", path);
		if (dump_dev(path)) {
			ish_found = 1;
			break;
		}
	}
	closedir(pci_dir);

	if (!ish_found)
		printf("No ISH device found...\n");

	return 0;
}
