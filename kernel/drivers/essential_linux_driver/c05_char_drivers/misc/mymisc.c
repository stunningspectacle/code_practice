#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/uaccess.h>

#define MYMISC_NAME "MEM4M"
#define MEM_ORDER 10

struct mymisc_ctx {
	unsigned int pfn;
	unsigned long phys_addr;
	unsigned long virt_addr;
	int read_done;
} ctx;

static int mymisc_open(struct inode *inode, struct file *file)
{
	file->private_data = &ctx;

	ctx.read_done = 0;

	return 0;
}

static ssize_t mymisc_read(struct file *file,
		char __user *buf, size_t size, loff_t *offset)
{
	struct mymisc_ctx *my = file->private_data;

	if (my->read_done)
		return 0;

	pr_info("%s: pfn: %u, phys: 0x%lx, virt: 0x%lx\n",
			__func__, my->pfn, my->phys_addr, my->virt_addr);

	if (copy_to_user(buf, &my->phys_addr, sizeof(unsigned long))) {
		pr_err("%s: copy to user error\n", __func__);
		return -EIO;
	}
	my->read_done = 1;

	return sizeof(unsigned long);
}

static ssize_t mymisc_write(struct file *file,
		const char __user *buf, size_t size, loff_t *offset)
{
	pr_info("%s: pfn: %u, phys: 0x%lx, virt: 0x%lx\n",
			__func__, ctx.pfn, ctx.phys_addr, ctx.virt_addr);

	return size;
}

static struct file_operations mymisc_fops = {
	.owner = THIS_MODULE,
	.open = mymisc_open,
	.read = mymisc_read,
	.write = mymisc_write,
};

static struct miscdevice mymisc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MYMISC_NAME,
	.fops = &mymisc_fops,
};

static int __init mymisc_init(void)
{
	int ret;
	struct page *page;

	if (misc_register(&mymisc)) {
		pr_err("Failed to register misc device\n");
		return -1;
	}
	pr_info("register misc device succeed\n");

	page = alloc_pages(GFP_KERNEL, MEM_ORDER);
	if (!page) {
		pr_err("failed to alloc mem\n");
		ret = -ENOMEM;
		goto err;
	}
	ctx.pfn = page_to_pfn(page);
	ctx.virt_addr = (unsigned long)pfn_to_kaddr(ctx.pfn);
	ctx.phys_addr = __pa(ctx.virt_addr); 
	pr_info("pfn: %u, phys: 0x%lx, virt: 0x%lx\n",
			ctx.pfn, ctx.phys_addr, ctx.virt_addr);

	return 0;

err:
	misc_deregister(&mymisc);
	return ret;
}

static void __exit mymisc_exit(void)
{
	free_pages(ctx.virt_addr, MEM_ORDER);
	misc_deregister(&mymisc);
}

module_init(mymisc_init);
module_exit(mymisc_exit);
MODULE_LICENSE("GPL");


