/*
 *
 * (C) COPYRIGHT 2014-2015 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */



#include <linux/device.h>

#include "malidp_hw.h"

#ifdef DEBUG
static ssize_t malidp_show_attr(struct device *dev, u32 attr, char *buf)
{
	struct malidp_device *dp_dev = dev_get_drvdata(dev);
	u32 value;
	int ret;

	ret = malidp_hw_get_attr(dp_dev->hw_dev, attr, &value);
	if (ret)
		return ret;

	return scnprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t malidp_store_attr(struct device *dev, u32 attr, const char *buf,
	size_t count)
{
	struct malidp_device *dp_dev = dev_get_drvdata(dev);
	u32 value = 0;
	int ret;

	if (!count)
		return 0;
	if (sscanf(buf, "%u", &value) != 1)
		return -EINVAL;

	mutex_lock(&dp_dev->adf_dev.client_lock);
	ret = malidp_hw_set_attr(dp_dev->hw_dev, attr, value);
	mutex_unlock(&dp_dev->adf_dev.client_lock);

	return ret ? ret : count;
}

#define MALIDP_DEVICE_ATTR(_name, _mode, _attr)				\
	static ssize_t show_##_name(struct device *dev,			\
				    struct device_attribute *attr,	\
				    char *buf)				\
	{								\
		return malidp_show_attr(dev, _attr, buf);		\
	}								\
	static ssize_t store_##_name(struct device *dev,		\
				     struct device_attribute *attr,	\
				     const char *buf, size_t count)	\
	{								\
		return malidp_store_attr(dev, _attr, buf, count);	\
	}								\
	static struct device_attribute dev_attr_##_name = {		\
		.attr = {						\
			.name = __stringify(_name),			\
			.mode = _mode,					\
		},							\
		.show   = show_##_name,					\
		.store  = store_##_name,				\
	}

MALIDP_DEVICE_ATTR(de_burstlen, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_BURSTLEN);
MALIDP_DEVICE_ATTR(de_poutstdcab, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_POUTSTDCAB);
MALIDP_DEVICE_ATTR(de_outstran, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_OUTSTRAN);
MALIDP_DEVICE_ATTR(de_rqos_low, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_RQOS_LOW);
MALIDP_DEVICE_ATTR(de_rqos_high, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_RQOS_HIGH);
MALIDP_DEVICE_ATTR(de_rqos_red, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_RQOS_RED);
MALIDP_DEVICE_ATTR(de_rqos_green, S_IWUSR | S_IRUGO, MALIDP_ATTR_DE_RQOS_GREEN);
MALIDP_DEVICE_ATTR(de_fifo_size, S_IRUGO, MALIDP_ATTR_DE_FIFO_SIZE);
MALIDP_DEVICE_ATTR(se_burstlen, S_IWUSR | S_IRUGO, MALIDP_ATTR_SE_BURSTLEN);
MALIDP_DEVICE_ATTR(se_outstran, S_IWUSR | S_IRUGO, MALIDP_ATTR_SE_OUTSTRAN);
MALIDP_DEVICE_ATTR(se_wcache, S_IWUSR | S_IRUGO, MALIDP_ATTR_SE_WCACHE);
MALIDP_DEVICE_ATTR(se_wqos, S_IWUSR | S_IRUGO, MALIDP_ATTR_SE_WQOS);

static struct attribute *malidp_debug_attrs[] = {
	&dev_attr_de_burstlen.attr,
	&dev_attr_de_poutstdcab.attr,
	&dev_attr_de_outstran.attr,
	&dev_attr_de_rqos_low.attr,
	&dev_attr_de_rqos_high.attr,
	&dev_attr_de_rqos_red.attr,
	&dev_attr_de_rqos_green.attr,
	&dev_attr_de_fifo_size.attr,
	&dev_attr_se_burstlen.attr,
	&dev_attr_se_outstran.attr,
	&dev_attr_se_wcache.attr,
	&dev_attr_se_wqos.attr,
	NULL,
};

static struct attribute_group malidp_debug_attr_group = {
	.attrs = malidp_debug_attrs,
};
#endif

static ssize_t show_hw_clock_ratio(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct malidp_device *dp_dev = dev_get_drvdata(dev);
	u32 clock_ratio = malidp_hw_clock_ratio_get(dp_dev->hw_dev);
	u32 ratio_i = (clock_ratio >> 16) & 0xFFFF;
	u32 ratio_f = 0;

	clock_ratio &= 0xFFFF;
	/* show clock_ratio as X.XX */
	if (clock_ratio != 0)
		ratio_f = (clock_ratio * 10000) >> 16;

	return snprintf(buf, PAGE_SIZE, "%u.%04u\n", ratio_i, ratio_f);
}

#define RATIO_BASE	10
static ssize_t store_hw_clock_ratio(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct malidp_device *dp_dev = dev_get_drvdata(dev);
	char str[5];
	u32 ratio = 0, fraction = 0, base = RATIO_BASE;
	int i;

	if (!count)
		return 0;

	str[0] = '\0';
	if (sscanf(buf, "%u.%4s", &ratio, str) > 0) {
		ratio = (ratio << 16) & 0xFFFF0000;

		for (i = 0; str[i] != '\0' && i < 5; i++) {
			if (str[i] < '0' || str[i] > '9')
				break;
			fraction += ((str[i] - '0') << 16) / base;
			base *= RATIO_BASE;
		}

		ratio += fraction;
		i = malidp_hw_clock_ratio_set(dp_dev->hw_dev, ratio);
		if (i != 0)
			return -EINVAL;
	}

	return count;
}

static ssize_t show_core_id(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct malidp_device *dp_dev = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%08x\n", dp_dev->core_id);
}

DEVICE_ATTR(clock_ratio, S_IWUSR | S_IRUGO,
		show_hw_clock_ratio, store_hw_clock_ratio);
DEVICE_ATTR(core_id, S_IRUGO,
	    show_core_id, NULL);

static struct attribute *malidp_hw_attrs[] = {
	&dev_attr_clock_ratio.attr,
	&dev_attr_core_id.attr,
	NULL
};

static struct attribute_group malidp_hw_attr_group = {
	.attrs = malidp_hw_attrs,
};

static struct attribute_group *malidp_attr_groups[] = {
#ifdef DEBUG
	&malidp_debug_attr_group,
#endif
	&malidp_hw_attr_group,
	NULL,
};


int malidp_sysfs_init(struct malidp_device *dev)
{
	struct attribute_group **group = malidp_attr_groups;
	attr_visible_t func = malidp_hw_get_attr_visible_func(dev->hw_dev);
	int ret = 0;

	while (*group) {
		(*group)->is_visible = func;
		ret = sysfs_create_group(&dev->device->kobj, *group);
		if (ret) {
			dev_err(dev->device, "failed registering attribute group %p", *group);
			break;
		}
		group++;
	}

	return ret;
}

void malidp_sysfs_destroy(struct malidp_device *dev)
{
	struct attribute_group **group = malidp_attr_groups;

	while (*group) {
		sysfs_remove_group(&dev->device->kobj, *group);
		group++;
	}
}
