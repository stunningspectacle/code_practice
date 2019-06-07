#include <linux/module.h>

#define MEMSIZE (1024 * 16)
static void *mem;
static int order;
static u64 sample_bytes = 0x12345678abcdefab;

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

	return 0;
}

static void __exit c11_exit(void)
{
	pr_info("%s is called\n", __func__);

	if (IS_ERR(mem))
		pr_info("mem was not alloc, err=%ld\n", PTR_ERR(mem));
	else {
		free_pages((unsigned long)mem, order);
		pr_info("mem was freed, mem(%p)\n", mem);
	}
}

module_init(c11_init);
module_exit(c11_exit);
MODULE_LICENSE("GPL");
