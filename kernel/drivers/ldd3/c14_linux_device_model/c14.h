#include <linux/module.h>

struct c14_device {
	int id;
	struct device dev;
	int value;
};

struct c14_driver {
	int id;
	struct device_driver drv;
	int (*probe)(struct c14_device *dev);
	void (*remove)(struct c14_device *dev);
};

int c14_device_register(struct c14_device *cdev);
void c14_device_unregister(struct c14_device *cdev);

int c14_driver_register(struct c14_driver *cdrv);
void c14_driver_unregister(struct c14_driver *cdrv);
