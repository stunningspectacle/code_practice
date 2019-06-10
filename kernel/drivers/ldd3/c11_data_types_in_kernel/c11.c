#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/list.h>

#define MEMSIZE (1024 * 16)

struct c11_struct {
	struct list_head entry;
	int id;
	int num;
};

static LIST_HEAD(c11_head);
static void *mem;
static int order;
static u64 sample_bytes = 0x12345678abcdefab;
static struct proc_dir_entry *proc;

static int c11_show(struct seq_file *m, void *data)
{
	int n = 0;
	struct c11_struct *c11;

	if (list_empty(&c11_head))
		seq_printf(m, "c11_head is empty\n");
	else {
		list_for_each_entry(c11, &c11_head, entry) {
			seq_printf(m, "id = %d, num = %d\n",
					c11->id, c11->num);
			n++;
		}
	}
	c11 = kmalloc(sizeof(struct c11_struct), GFP_KERNEL);	
	if (!c11) {
		seq_printf(m, "failed to alloc c11_struct");
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&c11->entry);
	c11->id = n;
	c11->num = n * n;
	list_add_tail(&c11->entry, &c11_head);
	seq_printf(m, "%s is called, id(%d), num(%d)\n",
			__func__, c11->id, c11->num);

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(c11);

static int __init c11_init(void)
{
	u64 tmp;

	pr_info("%s is called\n", __func__);
	order = get_order(MEMSIZE);
	mem = (void *)__get_free_pages(GFP_KERNEL, order);
	if (!mem) {
		mem = ERR_PTR(-ENOMEM);
		pr_err("failed to alloc mem with order %d\n", order);
	} else
		pr_info("mem alloc succeed @%p with order %d\n", mem, order);

	pr_info("sample bytes = 0x%llx\n", sample_bytes);
	tmp = cpu_to_be64(sample_bytes);
	pr_info("tmp = 0x%llx\n", tmp);
	tmp = be64_to_cpu(tmp);
	pr_info("tmp = 0x%llx\n", tmp);

	proc = proc_create("c11_proc", 0444, NULL, &c11_fops);
	if (!proc)
		pr_err("failed to create c11_proc\n");

	//INIT_LIST_HEAD(&c11_head);

	return 0;
}

static void __exit c11_exit(void)
{
	struct c11_struct *c11, *tmp;
	pr_info("%s is called\n", __func__);

	if (IS_ERR(mem))
		pr_info("mem was not alloc, err=%ld\n", PTR_ERR(mem));
	else {
		free_pages((unsigned long)mem, order);
		pr_info("mem was freed, mem(%p)\n", mem);
	}
	if (proc) {
		pr_info("Removing proc\n");
		proc_remove(proc);
	}
	if (list_empty(&c11_head))
		pr_info("c11_head is empty");
	else {
		pr_info("c11_head is not empty");
		list_for_each_entry_safe(c11, tmp, &c11_head, entry) {
			pr_info("id=%d, num=%d\n", c11->id, c11->num);
			list_del(&c11->entry);
			kfree(c11);
		}
	}
	if (list_empty(&c11_head))
		pr_info("c11_head is empty\n");
		
}

module_init(c11_init);
module_exit(c11_exit);
MODULE_LICENSE("GPL");
