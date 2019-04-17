#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
//#include <linux/printk.h>

#define myprintk(level, msg) printk(level msg #level level"\n")

#if 0
static int hello_proc_show(struct seq_file *m, void *v) {
	seq_printf(m, "Hello proc!\n");
	return 0;
}

static int hello_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, hello_proc_show, NULL);
}

static const struct file_operations hello_proc_fops = {
	.owner = THIS_MODULE,
	.open = hello_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif


static int my_proc_show(struct seq_file *seq, void *data)
{
	seq_printf(seq, "This is my proc hello!\n");

	return 0;
}

static int my_proc_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, my_proc_show, NULL);
}

static struct file_operations my_proc_fops = {
	.owner = THIS_MODULE,
	.open = my_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init hello_init(void)
{
	myprintk(KERN_DEBUG, "hello world");
	myprintk(KERN_INFO, "hello world");
	myprintk(KERN_NOTICE, "hello world");
	myprintk(KERN_WARNING, "hello world");
	myprintk(KERN_ERR, "hello world");
	myprintk(KERN_CRIT, "hello world");
	myprintk(KERN_ALERT, "hello world");
	myprintk(KERN_EMERG, "hello world");

	proc_create("hello_my", 0, NULL, &my_proc_fops);
	return 0;
}

static void __exit hello_exit(void)
{
	myprintk(KERN_DEBUG, "bye world");
	myprintk(KERN_INFO, "bye world");
	myprintk(KERN_NOTICE, "bye world");
	myprintk(KERN_WARNING, "bye world");
	myprintk(KERN_ERR, "bye world");
	myprintk(KERN_CRIT, "bye world");
	myprintk(KERN_ALERT, "bye world");
	myprintk(KERN_EMERG, "bye world");

	remove_proc_entry("hello_my", NULL);
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
