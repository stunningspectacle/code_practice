#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define C14NAME "c14"

static struct proc_dir_entry *proc;

static dev_t c14_dev;
static char c14_buf[1024];
static atomic_t is_read = ATOMIC_INIT(0);

static int c14_show(struct seq_file *s, void *data)
{
	seq_printf(s, "MAJOR(dev)=%d\n", MAJOR(c14_dev));

	return 0;
}

/*
static int c14_open(struct inode *inode, struct file *filp)
{
	return 0;
}
*/

static ssize_t c14_read(struct file *filp,
		char __user *buf, size_t size, loff_t *ppos)
{
	int n;

	if (atomic_cmpxchg(&is_read, 0, 1) == 0) {
		n = snprintf(c14_buf,
				sizeof(c14_buf), "MAJOR(dev)=%d\n", MAJOR(c14_dev));
		if (copy_to_user(buf, c14_buf, n)) {
			pr_err("Failed to copy to user buf\n");
			return -EFAULT;
		}
		return n;
	} 

	atomic_cmpxchg(&is_read, 1, 0);

	return 0;
}

static struct file_operations c14_fops = {
	.owner = THIS_MODULE,
	//.open = c14_open,
	.read = c14_read,
};

static struct miscdevice c14_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = C14NAME,
	.fops = &c14_fops,
	.mode = 0666,
};

static int __init c14_init(void)
{
	int ret;

	pr_info("%s is called\n", __func__);
	proc = proc_create_single(C14NAME, 0666, NULL, c14_show);
	if (proc)
		pr_info("proc entry is created\n");
	else
		pr_err("Failed to create proc entry\n");

	ret = alloc_chrdev_region(&c14_dev, 0, 1, C14NAME);
	if (ret) {
		pr_err("Failed to alloc dev id\n");
		return ret;
	}

	return misc_register(&c14_misc);
}

static void __exit c14_exit(void)
{
	pr_info("%s is called\n", __func__);

	if (proc) {
		proc_remove(proc);
		pr_info("proc is removed\n");
	}
	unregister_chrdev_region(c14_dev, 1);
	misc_deregister(&c14_misc);
}

module_init(c14_init);
module_exit(c14_exit);
MODULE_LICENSE("GPL");
