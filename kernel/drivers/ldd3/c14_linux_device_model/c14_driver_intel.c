#include <linux/module.h>
#include <linux/device.h>
#include "c14.h"

static int c14_intel_probe(struct c14_device *dev)
{
	pr_info("%s is called\n", __func__);
	return 0;
}

static void c14_intel_remove(struct c14_device *dev)
{
	pr_info("%s is called\n", __func__);
}

static struct c14_driver c14_intel = {
	.id = 0xaa,
	.drv = {
		.name = "intel",
	},
	.probe = c14_intel_probe,
	.remove = c14_intel_remove,
};

static int __init c14_driver_init(void)
{
	pr_info("%s is called\n", __func__);

	return c14_driver_register(&c14_intel);
}

static void __exit c14_driver_exit(void)
{
	pr_info("%s is called\n", __func__);
}

module_init(c14_driver_init);
module_exit(c14_driver_exit);
MODULE_LICENSE("GPL");

