#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include "c14.h"

#define C14_NAME_LEN 32

static int c14_dev_nr;

static ssize_t kk_show(struct bus_type *bus, char *buf)
{
	pr_info("%s is called\n", __func__);

	return 0;
}
static ssize_t kk_store(struct bus_type *bus, const char *buf, size_t count)
{
	pr_info("%s is called\n", __func__);

	return count;
}

BUS_ATTR_RW(kk);

static int c14bus_match(struct device *dev, struct device_driver *drv)
{
	pr_info("%s is called\n", __func__);

	return 0;
}

static int c14bus_dev_probe(struct device *dev)
{
	pr_info("%s is called\n", __func__);

	return 0;
}

static int c14bus_dev_remove(struct device *dev)
{
	pr_info("%s is called\n", __func__);

	return 0;
}

static struct bus_type c14_bus_type = {
	.name = "c14bus",
	.match = c14bus_match,
	.probe = c14bus_dev_probe,
	.remove = c14bus_dev_remove,
};

static void c14_device_release(struct device *dev)
{
	pr_info("%s is called\n", __func__);
}

int c14_device_register(struct c14_device *cdev)
{
	char *buf;

	pr_info("%s is called\n", __func__);

	device_initialize(&cdev->dev);
	cdev->dev.bus = &c14_bus_type;
	if (!cdev->dev.init_name) {
		buf = devm_kmalloc(&cdev->dev, C14_NAME_LEN, GFP_KERNEL);
		snprintf(buf, C14_NAME_LEN, "c14_dev-%d", c14_dev_nr);
		cdev->dev.init_name = buf;
	}
	c14_dev_nr++;

	if (!cdev->dev.release)
		cdev->dev.release = c14_device_release;

	return device_add(&cdev->dev);
}
EXPORT_SYMBOL_GPL(c14_device_register);

void c14_device_unregister(struct c14_device *cdev)
{
	pr_info("%s is called\n", __func__);

	device_unregister(&cdev->dev);
	c14_dev_nr--;
}
EXPORT_SYMBOL_GPL(c14_device_unregister);

static int c14_bus_driver_added(struct device *dev, void *data)
{
	struct c14_device *c14_dev = container_of(dev, struct c14_device, dev);
	struct c14_driver *c14_drv = data;

	if (c14_drv->id == c14_dev->id) {
		pr_info("good driver found for id(%d)\n", c14_drv->id);
		c14_drv->probe(c14_dev);
	}

	return 0;
}

int c14_driver_register(struct c14_driver *cdrv)
{
	pr_info("%s is called\n", __func__);

	cdrv->drv.bus = &c14_bus_type;
	driver_register(&cdrv->drv);
	
	return bus_for_each_dev(&c14_bus_type, NULL, cdrv, c14_bus_driver_added);
}
EXPORT_SYMBOL_GPL(c14_driver_register);

void c14_driver_unregister(struct c14_driver *cdrv)
{
	pr_info("%s is called\n", __func__);
	driver_unregister(&cdrv->drv);
}
EXPORT_SYMBOL_GPL(c14_driver_unregister);

static int __init c14_bus_init(void)
{
	pr_info("%s is called\n", __func__);

	bus_register(&c14_bus_type);
	bus_create_file(&c14_bus_type, &bus_attr_kk);

	return 0;
}

static void __exit c14_bus_exit(void)
{
	pr_info("%s is called\n", __func__);

	bus_unregister(&c14_bus_type);
}

module_init(c14_bus_init);
module_exit(c14_bus_exit);
MODULE_LICENSE("GPL");
