#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/proc_fs.h>

#define PROC_NAME "myblockio"

DECLARE_WAIT_QUEUE_HEAD(wait);
static int flag; 

static ssize_t blockio_read(struct file *filp,
		char __user *buf, size_t size, loff_t *offset)
{
	pr_info("%s: go to sleep\n", __func__);
	wait_event_interruptible(wait, flag != 0);
	flag = 0;
	pr_info("%s: I'm waken up!\n", __func__);

	return 0;
}

static ssize_t blockio_write(struct file *filp,
		const char __user *buf, size_t size, loff_t *offset)
{
	pr_info("%s: called\n", __func__);
	flag = 1;
	wake_up_interruptible(&wait);

	return size;
}

static struct file_operations blockio_fops = {
	.read = blockio_read,
	.write = blockio_write,
};

static int __init block_io_init(void)
{
	pr_info("%s called\n", __func__);

	proc_create(PROC_NAME, 0666, NULL, &blockio_fops);
	return 0;
}

static void __exit block_io_exit(void)
{
	pr_info("%s called\n", __func__);
	remove_proc_entry(PROC_NAME, NULL);
}

module_init(block_io_init);
module_exit(block_io_exit);
MODULE_LICENSE("GPL");
