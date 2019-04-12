#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/printk.h>

#define myprintk(level, msg) printk(level msg #level level"\n")
int __init hello_init(void)
{
	myprintk(KERN_DEBUG, "hello world");
	myprintk(KERN_INFO, "hello world");
	myprintk(KERN_NOTICE, "hello world");
	myprintk(KERN_WARNING, "hello world");
	myprintk(KERN_ERR, "hello world");
	myprintk(KERN_CRIT, "hello world");
	myprintk(KERN_ALERT, "hello world");
	myprintk(KERN_EMERG, "hello world");

	return 0;
}

void __exit hello_exit(void)
{
	myprintk(KERN_DEBUG, "bye world");
	myprintk(KERN_INFO, "bye world");
	myprintk(KERN_NOTICE, "bye world");
	myprintk(KERN_WARNING, "bye world");
	myprintk(KERN_ERR, "bye world");
	myprintk(KERN_CRIT, "bye world");
	myprintk(KERN_ALERT, "bye world");
	myprintk(KERN_EMERG, "bye world");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
