#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>

#define IOLEN	4
#define IOADDR	0x30
static struct proc_dir_entry *proc_entry;

static int c09test_show(struct seq_file *m, void *data)
{
	seq_printf(m, "this is test\n");
	smp_mb();
	pr_info("m->size:(%ld), m->count(%ld)\n", m->size, m->count);

	return 0;
}

static int c09test_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, c09test_show, NULL);
}

#if 0
static ssize_t c09test_read(struct file *filp,
		char __user *buf, size_t len, loff_t *offset)
{
	return len;
}
#endif

static ssize_t c09test_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *offset)
{
	struct seq_file *m = filp->private_data;

	if (m->count + len > m->size)
		return -EINVAL;
	if (copy_from_user(m->buf, buf, len))
		return -EFAULT;

	return len;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = seq_read,
	.write = c09test_write,
	.open = c09test_open,
	.release = seq_release,
};

static int __init c09test_init(void)
{
	pr_info("%s called\n", __func__);
	proc_entry = proc_create("c09test", 0777, NULL, &fops);

	if (!request_region(IOADDR, IOLEN, "c09test")) {
		pr_err("Failed to request io resource\n");
		return -EBUSY;
	}

	return 0;
}

static void __exit c09test_exit(void)
{
	pr_info("%s called\n", __func__);
	proc_remove(proc_entry);
	release_region(IOADDR, IOLEN);
}

module_init(c09test_init);
module_exit(c09test_exit);
MODULE_LICENSE("GPL");
