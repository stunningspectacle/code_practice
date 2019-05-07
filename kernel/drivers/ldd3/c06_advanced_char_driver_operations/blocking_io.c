#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/init.h>

#define PROC_NAME "myblockio"
#define MISC_NAME "mymisc"
#define BUFSIZE 1024

struct blocking_io_dev {
	char block_buf[BUFSIZE];
	int count;
	wait_queue_head_t wq;
	struct mutex mutex;
};

struct blocking_io_dev *blockio_dev;

//DECLARE_WAIT_QUEUE_HEAD(wait);
//static atomic_t flag = ATOMIC_INIT(0);
static unsigned int blockio_poll(struct file *filp, poll_table *table)
{
	unsigned int mask = 0;

	mutex_lock(&blockio_dev->mutex);
	poll_wait(filp, &blockio_dev->wq, table);

	if (blockio_dev->count > 0)
		mask |= POLLIN | POLLRDNORM;

	mutex_unlock(&blockio_dev->mutex);

	return mask;
}

static int blockio_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t blockio_read(struct file *filp,
		char __user *buf, size_t size, loff_t *offset)
{
	pr_info("%s: called\n", __func__);

	if (mutex_lock_interruptible(&blockio_dev->mutex))
		return -ERESTARTSYS;

	while (!blockio_dev->count) {
		mutex_unlock(&blockio_dev->mutex);
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		pr_info("%s: go to sleep\n", __func__);
		wait_event_interruptible(blockio_dev->wq,
				blockio_dev->count != 0);
		pr_info("%s: I'm waken up!\n", __func__);
		if (mutex_lock_interruptible(&blockio_dev->mutex))
			return -ERESTARTSYS;
	}
	if (copy_to_user(buf, blockio_dev->block_buf, blockio_dev->count)) {
		pr_err("failed to copy data to user\n");
		mutex_unlock(&blockio_dev->mutex);
		return -EFAULT;
	}
	blockio_dev->count = 0;
	mutex_unlock(&blockio_dev->mutex);

	return blockio_dev->count;
}

static ssize_t blockio_write(struct file *filp,
		const char __user *buf, size_t size, loff_t *offset)
{
	pr_info("%s: called\n", __func__);
	if (mutex_lock_interruptible(&blockio_dev->mutex))
		return -ERESTARTSYS;
	
	if (copy_from_user(&blockio_dev->block_buf[blockio_dev->count], buf, size)) {
		mutex_unlock(&blockio_dev->mutex);
		pr_err("failed to copy data from user\n");
		return -EFAULT;
	}
	blockio_dev->count += size;
	mutex_unlock(&blockio_dev->mutex);

	wake_up_interruptible(&blockio_dev->wq);

	return size;
}

static struct file_operations blockio_fops = {
	.read = blockio_read,
	.write = blockio_write,
	.open = blockio_open,
	.poll = blockio_poll,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MISC_NAME,
	.fops = &blockio_fops,
};

static int __init blockio_init(void)
{
	pr_info("%s called\n", __func__);

	blockio_dev = kzalloc(sizeof(*blockio_dev), GFP_KERNEL);
	if (!blockio_dev) {
		pr_err("Failed to alloc mem\n");
		return -ENOMEM;
	}

	init_waitqueue_head(&blockio_dev->wq);
	mutex_init(&blockio_dev->mutex);

	if (misc_register(&misc)) {
		pr_err("failed to register miscdevice\n");
		return -EFAULT;
	}
	pr_info("miscdevice registered\n");

	proc_create(PROC_NAME, 0666, NULL, &blockio_fops);
	return 0;
}

static void __exit blockio_exit(void)
{
	pr_info("%s called\n", __func__);
	if (blockio_dev)
		kfree(blockio_dev);
	misc_deregister(&misc);
	remove_proc_entry(PROC_NAME, NULL);
}

module_init(blockio_init);
module_exit(blockio_exit);
MODULE_LICENSE("GPL");
