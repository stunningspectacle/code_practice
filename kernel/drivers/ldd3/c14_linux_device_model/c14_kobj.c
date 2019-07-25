#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/slab.h>

struct c14_sample {
	int value;
	struct kobject kobj;
};

struct c14_sample *sample0;
struct c14_sample *sample1;

struct c14_sample *sample2;
struct c14_sample *sample3;

struct kset *c14_kset;

static struct attribute attr_name = {
	.name = "name",
	.mode = 0444,
};
static struct attribute attr_value = {
	.name = "value",
	.mode = 0666,
};

static struct attribute *c14_attrs[] = {
	&attr_name,
	&attr_value,
	NULL,
};

static ssize_t sample_show(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	ssize_t size = 0;
	struct c14_sample *sample = container_of(kobj, struct c14_sample, kobj);

	if (!strcmp(attr->name, "name"))
		size = sprintf(buf, "%s", kobject_name(kobj));
	else if (!strcmp(attr->name, "value"))
		size = sprintf(buf, "%d", sample->value);

	return size;
}

static ssize_t sample_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t size)
{
	struct c14_sample *sample = container_of(kobj, struct c14_sample, kobj);

	if (!strcmp(attr->name, "value"))
		sscanf(buf, "%d", &sample->value);

	return size;
}

static void c14_kobj_release(struct kobject *kobj)
{
	struct c14_sample *sample = container_of(kobj, struct c14_sample, kobj);
	pr_info("sample %s has value %d\n", kobj->name, sample->value);
	kfree(sample);
}

static struct sysfs_ops c14_sysfs_ops = {
	.show  = sample_show,
	.store = sample_store,
};

static struct kobj_type c14_ktype = {
	.release = c14_kobj_release,
	.sysfs_ops = &c14_sysfs_ops,
	.default_attrs = c14_attrs,
};

static int __init c14_init(void)
{
	pr_info("%s is called\n", __func__);

	sample0 = kzalloc(sizeof(struct c14_sample), GFP_KERNEL);	
	sample1 = kzalloc(sizeof(struct c14_sample), GFP_KERNEL);

	kobject_init_and_add(&sample0->kobj, &c14_ktype, NULL, "c14_kobj_sample0");
	kobject_init_and_add(&sample1->kobj, &c14_ktype, &sample0->kobj, "c14_kobj_sample1");

	sample2 = kzalloc(sizeof(struct c14_sample), GFP_KERNEL);	
	sample3 = kzalloc(sizeof(struct c14_sample), GFP_KERNEL);

	c14_kset = kset_create_and_add("c14_kset_samples", NULL, NULL);	
	sample2->kobj.kset = c14_kset;
	sample3->kobj.kset = c14_kset;
	kobject_init_and_add(&sample2->kobj, &c14_ktype, NULL, "c14_sample2");
	kobject_init_and_add(&sample3->kobj, &c14_ktype, NULL, "c14_sample3");
	
	return 0;
}

static void __exit c14_exit(void)
{
	pr_info("%s is called\n", __func__);
	kobject_del(&sample0->kobj);
	kobject_put(&sample0->kobj);

	kobject_del(&sample1->kobj);
	kobject_put(&sample1->kobj);

	kobject_del(&sample2->kobj);
	kobject_put(&sample2->kobj);

	kobject_del(&sample3->kobj);
	kobject_put(&sample3->kobj);

	kset_unregister(c14_kset);
}

module_init(c14_init);
module_exit(c14_exit);
MODULE_LICENSE("GPL");
