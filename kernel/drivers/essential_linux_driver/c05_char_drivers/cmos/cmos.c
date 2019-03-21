#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define NUM_CMOS_BANKS          2
#define CMOS_BANK_SIZE          (0xFF*8)
#define DEVICE_NAME             "cmosx"
#define CMOS_BANK0_INDEX_PORT   0x90
#define CMOS_BANK0_DATA_PORT    0x91

#define CMOS_BANK1_INDEX_PORT   0x92
#define CMOS_BANK1_DATA_PORT    0x93

#define DATA_SIZE 50

#define CMOS_ADJUST_CHECKSUM	1
#define CMOS_VERIFY_CHECKSUM	2

struct cmos_dev {
	unsigned short current_pointer;
	unsigned int size;
	int bank_number;
	struct cdev cdev;
	char data[DATA_SIZE];
	char name[20];
} *cmos_devp[NUM_CMOS_BANKS];

static dev_t cmos_dev_number;
struct class *cmos_class;

unsigned char addrports[NUM_CMOS_BANKS] = {
	CMOS_BANK0_INDEX_PORT,
	CMOS_BANK1_INDEX_PORT
};
unsigned char dataports[NUM_CMOS_BANKS] = {
	CMOS_BANK0_DATA_PORT,
	CMOS_BANK1_DATA_PORT
};

static int cmos_open(struct inode *inode, struct file *file)
{
	struct cmos_dev *cmos;

	pr_info("%s called\n", __func__);
	cmos = container_of(inode->i_cdev, struct cmos_dev, cdev);
	file->private_data = cmos;

	cmos->size = DATA_SIZE;
	cmos->current_pointer = 0;

	return 0;
}

static int cmos_release(struct inode *inode, struct file *file)
{
	struct cmos_dev *cmos;

	pr_info("%s called\n", __func__);

	cmos = file->private_data;
	cmos->current_pointer = 0;

	return 0;
}

static ssize_t cmos_read(struct file *file,
		char __user *buf, size_t size, loff_t *offset)
{
	struct cmos_dev *cmos;
	ssize_t len = 0;

	pr_info("%s called and want to copy %ld bytes\n", __func__, size);

	cmos = file->private_data;
	if (!cmos->size)
		return 0;
	len = size > cmos->size ? cmos->size : size;
	pr_info("%s will copy %ld bytes\n", __func__, len);
	if (copy_to_user(buf, cmos->data, len)) {
		pr_err("%s: copy_to_user error!\n", __func__);
		return -EIO;
	}
	cmos->size -= len;

	return len;
}

static ssize_t cmos_write(struct file *file,
		const char __user *buf, size_t size, loff_t *offset)
{
	struct cmos_dev *cmos;
	size_t len = size > DATA_SIZE ? DATA_SIZE : size;

	pr_info("%s called and want to write %ld bytes\n", __func__, size);
	cmos = file->private_data;

	if (copy_from_user(cmos->data, buf, len))
		return -EIO;

	cmos->size = len;
	pr_info("%s write %ld bytes\n", __func__, len);

	return len;
}

static loff_t cmos_llseek(struct file *file, loff_t offset, int whence)
{
	struct cmos_dev *cmos = file->private_data;

	switch (whence) {
	case SEEK_CUR:
		cmos->current_pointer += offset;
		break;
	case SEEK_SET:
		cmos->current_pointer = offset;
		break;
	case SEEK_END:
		cmos->current_pointer = DATA_SIZE + offset;
		break;
	default:
		pr_err("invalid param\n");
		return -EINVAL;
	}
	pr_info("set offset to %d\n", cmos->current_pointer);
	return cmos->current_pointer;
}

static long cmos_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case CMOS_ADJUST_CHECKSUM:
		break;
	case CMOS_VERIFY_CHECKSUM:
		break;
	default:
		pr_err("invalid arg\n");
		return -EINVAL;
	}

	return 0;
}

static struct file_operations cmos_fops = {
	.owner = THIS_MODULE,
	.open = cmos_open,
	.release = cmos_release,
	.read = cmos_read,
	.write = cmos_write,
	.llseek = cmos_llseek,
	.unlocked_ioctl = cmos_ioctl,
};

int __init cmos_init(void)
{
	int i;
	int ret = 0;

	if (alloc_chrdev_region(&cmos_dev_number,
				0, NUM_CMOS_BANKS, DEVICE_NAME) < 0) {
		pr_err("alloc_chrdev_region failed!\n");
		return -1;
	}

	pr_info("alloc_chrdev_region done!\n");
	cmos_class = class_create(THIS_MODULE, DEVICE_NAME);
	for (i = 0; i < NUM_CMOS_BANKS; i++) {
		cmos_devp[i] = kzalloc(sizeof(struct cmos_dev), GFP_KERNEL);
		if (!cmos_devp[i]) {
			pr_err("Failed to malloc mem\n");
			ret = -ENOMEM;
			goto err;
		}

		pr_info("kmalloc ok %d!\n", i);
		snprintf(cmos_devp[i]->name,
				sizeof(cmos_devp[i]->name), "%s%d", DEVICE_NAME, i);
		snprintf(cmos_devp[i]->data,
				DATA_SIZE,	
				"init data in %s%d", DEVICE_NAME, i);
		if (!request_region(addrports[i], 2, cmos_devp[i]->name)) {
			pr_err("I/O port 0x%x is not free\n", addrports[i]);
			ret = -EIO;
			goto err;
		}
		pr_info("request_region ok %d!\n", i);
		cmos_devp[i]->bank_number = i;

		cdev_init(&cmos_devp[i]->cdev, &cmos_fops);
		cmos_devp[i]->cdev.owner = THIS_MODULE;

		if (cdev_add(&cmos_devp[i]->cdev,
					MKDEV(MAJOR(cmos_dev_number), i), 1) < 0) {
			pr_err("failed to add cdev\n");
			ret = -1;
			goto err;
		}
		pr_info("cdev_add ok %d!\n", i);
		device_create(cmos_class,
				NULL, cmos_dev_number + i, NULL, "cmos%d", i);
		pr_info("device_create ok %d!\n", i);
	}

	pr_info("CMOS driver init finished\n");
	return ret;

err:
	if (cmos_dev_number)
		unregister_chrdev_region(MAJOR(cmos_dev_number),
				NUM_CMOS_BANKS);
	for (i = 0; i < NUM_CMOS_BANKS; i++) {
		if (cmos_devp[i]) {
			cdev_del(&cmos_devp[i]->cdev);
			device_destroy(cmos_class,
					MKDEV(MAJOR(cmos_dev_number), i));
			release_region(addrports[i], 2);
			kfree(cmos_devp[i]);
		}
	}
	if (cmos_class)
		class_destroy(cmos_class);

	return ret;
}

void __exit cmos_cleanup(void)
{
	int i;

	for (i = 0; i < NUM_CMOS_BANKS; i++) {
		cdev_del(&cmos_devp[i]->cdev);
		device_destroy(cmos_class, MKDEV(MAJOR(cmos_dev_number), i));
		release_region(addrports[i], 2);
	}
	unregister_chrdev_region(MAJOR(cmos_dev_number), NUM_CMOS_BANKS);
	class_destroy(cmos_class);

	pr_info("CMOS driver cleanup\n");
}

module_init(cmos_init);
module_exit(cmos_cleanup);
MODULE_LICENSE("GPL");
