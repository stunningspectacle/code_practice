#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/parport.h>

static dev_t dev_number;
static struct class *led_class;
struct cdev led_cdev;
struct pardevice *pdev;
struct kobject kobj;

struct led_attr {
	struct attribute attr;
	ssize_t (*show)(char *);
	ssize_t (*store)(const char *, size_t);
};

#define glow_show_led(number)						\
static ssize_t glow_led_##number(const char *buffer, size_t count)	\
{									\
	unsigned char buf; 						\
	int value;							\
	sscanf(buffer, "%d", &value);					\
	parport_claim_or_block(pdev);				\
	buf = parport_read_data(pdev->port);				\
	if (value)							\
		parport_write_data(pdev->port, buf | (1 << number));	\
	else								\
		parport_write_data(pdev->port, buf & ~(1 << number));	\
	parport_release(pdev);						\
	return count;							\
}									\
static ssize_t show_led_##number(char *buffer)				\
{									\
	unsigned char buf;						\
	parport_claim_or_block(pdev);					\
	buf = parport_read_data(pdev->port);				\
	parport_release(pdev);						\
	if (buf & (1 << number))					\
		return sprintf(buffer, "ON\n");				\
	else								\
		return sprintf(buffer, "OFF\n");			\
}									\
static struct led_attr led##number = 					\
__ATTR(led##number, 0644, show_led_##number, glow_led_##number);

glow_show_led(0);
glow_show_led(1);
glow_show_led(2);
glow_show_led(3);
glow_show_led(4);
glow_show_led(5);
glow_show_led(6);
glow_show_led(7);

#define DEVICE_NAME "led"

static int led_preempt(void *handle)
{
	return 1;
}

static void led_attach(struct parport *port)
{
	pdev = parport_register_device(port,
			DEVICE_NAME, led_preempt, NULL, NULL, 0, NULL);
	if (pdev == NULL)
		printk("Bad register\n");
}

static ssize_t l_show(struct kobject *kobj, struct attribute *a, char *buf)
{
	int ret;
	struct led_attr *lattr = container_of(a, struct led_attr,attr);
	ret = lattr->show ? lattr->show(buf) : -EIO;
	return ret;
}

static ssize_t l_store(struct kobject *kobj,
		struct attribute *a, const char *buf, size_t count)
{
	int ret;
	struct led_attr *lattr = container_of(a, struct led_attr, attr);
	ret = lattr->store ? lattr->store(buf, count) : -EIO;
	return ret;
}

static struct sysfs_ops sysfs_ops = {
	.show  = l_show,
	.store = l_store,
};

static struct attribute *led_attrs[] = {
	&led0.attr,
	&led1.attr,
	&led2.attr,
	&led3.attr,
	&led4.attr,
	&led5.attr,
	&led6.attr,
	&led7.attr,
	NULL
};

static struct kobj_type ktype_led = {
	.sysfs_ops = &sysfs_ops,
	.default_attrs = led_attrs,
};

static struct parport_driver led_driver = {
	.name = "led",
	.attach = led_attach,
};

int __init led_init(void)
{
	struct device *c_d;

	alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);

	printk("%s: %d.\n", __func__, __LINE__);
	led_class = class_create(THIS_MODULE, "pardevice");
	if (IS_ERR(led_class))
		printk("Bad class create\n");
	printk("%s: %d.\n", __func__, __LINE__);
	c_d = device_create(led_class, NULL, dev_number, NULL, DEVICE_NAME);
	if (parport_register_driver(&led_driver)) {
		pr_err("Bad Parport Register\n");
		return -EIO;
	}

	printk("%s: %d.\n", __func__, __LINE__);
	kobject_init(&kobj, &ktype_led);
	kobject_add(&kobj, &c_d->kobj, "control");
	printk("LED Driver Initialized.\n");

	return 0;
}

void led_cleanup(void)
{
	kobject_del(&kobj);
	device_destroy(led_class, MKDEV(MAJOR(dev_number), 0));
	class_destroy(led_class);

	return;
}

module_init(led_init);
module_exit(led_cleanup);
MODULE_LICENSE("GPL");


#if 0
// Template to add attribute to a device
cls = class_create();
dev = device_create(cls...);
kobject_init(&kobj, &ktype);
kobject_add(&kobj, &dev->kobj, NAME);

struct my_attr {
	struct attr attr;
	ssize_t (*show)(char *);
	ssize_t (*store)(const char *, size_t);
};
struct my_attr attr0 = __ATTR("attr0", 0644, myshow, mystore);

struct kobj_type ktype = {
	.sysfs_ops = {
	}
	.default_attrs = {
		attr0.attr,
		NULL,
	}
kobject_init(&kobj, &ktype);
kobject_add(&kobj, &parent_kobj, "kobj_for_attr");
#endif
