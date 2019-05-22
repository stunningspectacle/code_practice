#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/gfp.h>

struct hello_block {
	int a;
	int b;
	int c;
	char *pages;
};

struct hello_block *bs[10];
static int num;
struct proc_dir_entry *dir_entry;
struct kmem_cache *hello_cache;

static int hello_show(struct seq_file *seq_file, void *data)
{
	int i;
	struct hello_block *b;
	
	b = kmem_cache_alloc(hello_cache, GFP_KERNEL);
	if (!b)
		pr_err("Failed to alloc hello block");

	bs[num++] = b;
	b->a = num - 1;
	b->b = b->a * 2;
	b->c = b->a * 3;
	b->pages = (void *)__get_free_pages(GFP_KERNEL, 3);
	if (!b->pages)
		pr_err("Failed to alloc pages\n");
	else {
		for (i = 0; i < 100; i++)
			b->pages[i] = '0' + num;
		b->pages[i] = 0;
	}

	seq_printf(seq_file, "%d kmem_cache is allocated, %s\n", num, b->pages);

	return 0;
}

static int hello_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, hello_show, NULL);
}

static ssize_t hello_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *offset)
{
	struct seq_file *m = filp->private_data;

	pr_info("%s is called\n", __func__);

	seq_write(m, buf, len);

	return len;
}

struct file_operations fops = {
	.open = hello_open,
	.read = seq_read,
	.write = hello_write,
	.release = seq_release,
};

static int __init hello_init(void)
{
	pr_info("%s is called\n", __func__);

	dir_entry = proc_create("hello_proc", 0777, NULL, &fops);

	hello_cache = kmem_cache_create("hello_cache",
			sizeof(struct hello_block), 0, 0, NULL);
	if (!hello_cache) {
		pr_err("Failed to create hello cache\n");
		return -ENOMEM;
	}
	pr_info("create hello cache successfully\n");

	return 0;
}

static void __exit hello_exit(void)
{
	int i;
	struct hello_block *b;

	if (dir_entry)
		proc_remove(dir_entry);
	if (hello_cache)
		kmem_cache_destroy(hello_cache);
	for (i = 0; i < num; i++) {
		b = bs[i];
		pr_info("%d, %d, %d\n", b->a, b->b, b->c);
		if (b->pages) {
			pr_info("%s\n", b->pages);
			free_pages((unsigned long)b->pages, 3);
		}

	}
		
	pr_info("%s is called\n", __func__);
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
