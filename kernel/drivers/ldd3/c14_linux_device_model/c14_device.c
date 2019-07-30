#include <linux/module.h>
#include <linux/device.h>
#include "c14.h"

static struct c14_device c14_dev0;
static struct c14_device c14_dev1;

static int __init c14_device_init(void)
{
	pr_info("%s is called\n", __func__);

	c14_dev0.id = 0xaa;
	c14_device_register(&c14_dev0);

	c14_dev1.id = 0xbb;
	c14_device_register(&c14_dev1);

	return 0;
}

static void __exit c14_device_exit(void)
{
	pr_info("%s is called\n", __func__);
	c14_device_unregister(&c14_dev0);
	c14_device_unregister(&c14_dev1);
}

module_init(c14_device_init);
module_exit(c14_device_exit);
MODULE_LICENSE("GPL");
