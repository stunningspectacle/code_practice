/*
 *
 * (C) COPYRIGHT 2014 ARM Limited. All rights reserved.
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



/*
 * Global list containing all the video_tx_device registered with the
 * framework and a mutext to protect all accesses.
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_i2c.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include <video/video_tx.h>

#define DEFAULT_POLLING_PERIOD_MS 500
#define WQ_MAX_NAME 50

static DEFINE_MUTEX(client_mutex);
static LIST_HEAD(video_tx_list);
static struct workqueue_struct *video_tx_wq;
static u16 polling_period_ms = DEFAULT_POLLING_PERIOD_MS;

struct video_tx_device {
	struct list_head list;
	struct device *dev;
	const struct video_tx_driver *driver;
	/* Protects the notifier block for adding/removing listeners */
	struct mutex mutex;
	struct blocking_notifier_head notifier;
	struct delayed_work polling_dwork;
	enum drm_connector_status cached_status;
	bool polling_enabled;
	void *private;
};

/*
 * Search the "video_tx_list" to get a "struct video_tx_device"
 * whose "dev" field is equal to the argument of the function.
 */
static struct video_tx_device *find_video_tx_by_dev(struct device *dev)
{
	struct video_tx_device *tx_device;

	mutex_lock(&client_mutex);
	list_for_each_entry(tx_device, &video_tx_list, list) {
		if (tx_device->dev == dev) {
			mutex_unlock(&client_mutex);
			return tx_device;
		}
	}
	mutex_unlock(&client_mutex);

	return NULL;
}

/** Called from the host driver **/

/**
 * of_find_video_tx_by_node - find a video_tx from a device tree node
 *
 * @node: device tree node of the video transmitter
 *
 * This function will be called by the host in order to get a reference
 * to the video transmitter structure.
 *
 */
struct video_tx_device *of_find_video_tx_by_node(struct device_node *node)
{
	struct i2c_client *i2c;
	struct platform_device *pdev;
	struct device *dev;

	/* The device can be either in the i2c bus or in the platform bus */
	i2c = of_find_i2c_device_by_node(node);
	pdev = of_find_device_by_node(node);
	if (i2c)
		dev = &i2c->dev;
	else if (pdev)
		dev = &pdev->dev;
	else {
		pr_err("%s: could not find a device matching the dt node\n",
			__func__);
		return NULL;
	}

	return find_video_tx_by_dev(dev);
}
EXPORT_SYMBOL_GPL(of_find_video_tx_by_node);

/**
 * video_tx_hotplug_notifier_register - register hotplug notifier
 *
 * @video_tx: pointer to the video tx device_node
 * @nb: pointer to the notifier block provided by the host
 *
 * This function will be called by the host in order to show that it is
 * interested on receiving hotplug notifications from the client.
 *
 * It cannot be called from an atomic context.
 *
 */
void video_tx_hotplug_notifier_register(struct video_tx_device *video_tx,
					struct notifier_block *nb)
{
	mutex_lock(&video_tx->mutex);
	blocking_notifier_chain_register(&video_tx->notifier, nb);
	mutex_unlock(&video_tx->mutex);
}
EXPORT_SYMBOL_GPL(video_tx_hotplug_notifier_register);

/**
 * video_tx_hotplug_notifier_unregister - unregister hotplug notifier
 *
 * @video_tx: pointer to the video tx device_node
 * @nb: pointer to the notifier block provided by the host
 *
 * This function will be called by the host in order to show that it's
 * no longer interested on receiving hotplug notifications from the client.
 *
 * It cannot be called from an atomic context.
 *
 */
void video_tx_hotplug_notifier_unregister(struct video_tx_device *video_tx,
					struct notifier_block *nb)
{
	mutex_lock(&video_tx->mutex);
	blocking_notifier_chain_unregister(&video_tx->notifier, nb);
	mutex_unlock(&video_tx->mutex);
}
EXPORT_SYMBOL_GPL(video_tx_hotplug_notifier_unregister);

/**
 * video_tx_get_modes - get a list of the modes supported by a video tx
 *
 * @video_tx: pointer to the video tx device_node
 * @modelist: list of modes to be fill in by the client
 * @n: if modelist is not NULL, the number of modes there is space for
 *
 * This function will be called by the host in order to get the list of
 * modes supported by the video transmitter.
 *
 * If modelist is not NULL the client will write at most @a n modes into
 * the memory area pointed by this variable. If the returned value does not
 * match @a n, then the available mode list has changed, and the host should
 * repeat the process to get the correct modelist.
 *
 * The function will return the number of supported modes or negative on error.
 */
int video_tx_get_modes(struct video_tx_device *video_tx,
		       struct drm_mode_modeinfo *modelist, int n)
{
	if (!video_tx->driver->get_modes)
		return -EINVAL;

	return video_tx->driver->get_modes(video_tx, modelist, n);
}
EXPORT_SYMBOL_GPL(video_tx_get_modes);

/**
 * video_tx_set_mode - set the mode of a video tx
 *
 * @video_tx: pointer to the video tx device
 * @mode: the mode we want to set in the video tx
 *
 * This function will be called by a host in order to set the mode of a
 * client video tx.
 *
 * It will return 0 on success and negative if the mode cannot be set.
 *
 */
int video_tx_set_mode(struct video_tx_device *video_tx,
				struct drm_mode_modeinfo *mode)
{
	if (!video_tx->driver->set_mode)
		return -EINVAL;

	return video_tx->driver->set_mode(video_tx, mode);
}
EXPORT_SYMBOL_GPL(video_tx_set_mode);

/**
 * video_tx_dpms - set the power status of a video tx
 *
 * @video_tx: pointer to the video tx device
 * @mode: the power status we want to put the device into
 *	(follows the  DRM_MODE_DPMS definitions)
 *
 * This function will be called by a host in order to change the power
 * mode of a video tx client.
 *
 * It will return 0 on success and negative on error.
 *
 */
int video_tx_dpms(struct video_tx_device *video_tx, int mode)
{
	if (!video_tx->driver->dpms)
		return -EINVAL;

	return video_tx->driver->dpms(video_tx, mode);
}
EXPORT_SYMBOL_GPL(video_tx_dpms);

/**
 * video_tx_get_info - get the platform specific information from the video TX
 *
 * @video_tx: pointer to the video tx device
 * @info: pointer to the video_tx_info structure that will be filled in by the
 *	client video tx.
 *
 * It will return 0 on success and negative on error.
 *
 */
int video_tx_get_info(struct video_tx_device *video_tx,
		      struct video_tx_info *info)
{
	if (!video_tx->driver->get_tx_info)
		return -EINVAL;

	return video_tx->driver->get_tx_info(video_tx, info);
}
EXPORT_SYMBOL_GPL(video_tx_get_info);

/**
 * video_tx_get_display_info - get the display specific information from the TX
 *
 * @video_tx: pointer to the video tx device
 * @info: pointer to the video_tx_display_info structure that will be filled in
 *	by the client video tx.
 *
 * It will return 0 on success and negative on error.
 *
 */
int video_tx_get_display_info(struct video_tx_device *video_tx,
		      struct video_tx_display_info *info)
{
	if (!video_tx->driver->get_display_info)
		return -EINVAL;

	return video_tx->driver->get_display_info(video_tx, info);
}
EXPORT_SYMBOL_GPL(video_tx_get_display_info);

/**
 * video_tx_detect - get the current status of the video transmitter
 *
 * @video_tx: pointer to the video tx device
 *
 * This function will be called by a host that is interested on knowing
 * the hotplug status of a video transmitter.
 *
 */
enum drm_connector_status video_tx_detect(struct video_tx_device *video_tx)
{
	if (!video_tx->driver->detect)
		return -EINVAL;

	return video_tx->driver->detect(video_tx);
}
EXPORT_SYMBOL_GPL(video_tx_detect);

/** Called from the video transmitter client **/

/**
 * video_tx_report_hotplug_event - report a hotplug event to the framework
 *
 * @tx_device: pointer to the video tx device
 *
 * This function will be called by the video transmitter client when a new
 * hotplug event occurs. The framework will then signal the notifier so that
 * any hosts listening can get their notifier callbacks executed.
 *
 * Cannot be called from an atomic context.
 */
void video_tx_report_hotplug_event(struct video_tx_device *tx_device)
{
	enum drm_connector_status s;

	if (!tx_device->driver->detect) {
		dev_warn(tx_device->dev, "detect() not implemented\n");
		return;
	}

	s = tx_device->driver->detect(tx_device);

	blocking_notifier_call_chain(&tx_device->notifier, s, tx_device);
}
EXPORT_SYMBOL_GPL(video_tx_report_hotplug_event);

static void video_tx_polling_work(struct work_struct *work)
{
	struct video_tx_device *tx_device =
		container_of(work, struct video_tx_device, polling_dwork.work);
	enum drm_connector_status s = tx_device->driver->detect(tx_device);

	mutex_lock(&tx_device->mutex);
	if (s != tx_device->cached_status) {
		tx_device->cached_status = s;
		blocking_notifier_call_chain(&tx_device->notifier, s,
					     tx_device);
	}

	/* Queue work for the next period unless polling is disabled */
	if (tx_device->polling_enabled)
		queue_delayed_work(video_tx_wq, &tx_device->polling_dwork,
				   msecs_to_jiffies(polling_period_ms));
	mutex_unlock(&tx_device->mutex);
}

/**
 * video_tx_request_polling - use polling for hotplugs instead of interrupts
 *
 * @tx_device: pointer to the video tx device
 *
 * This function can be called by the video_tx client if this does not support
 * hotplug interrupts. The video tx framework will launch a kernel thread that
 * will perform polling instead by periodically calling detect() in the client.
 *
 * Cannot be called from an atomic context.
 */
void video_tx_request_polling(struct video_tx_device *tx_device)
{
	if (!tx_device->driver->detect) {
		dev_warn(tx_device->dev, "detect() not implemented\n");
		return;
	}

	mutex_lock(&tx_device->mutex);
	if (!tx_device->polling_enabled) {
		tx_device->cached_status =
			tx_device->driver->detect(tx_device);

		queue_delayed_work(video_tx_wq, &tx_device->polling_dwork,
				msecs_to_jiffies(polling_period_ms));
		tx_device->polling_enabled = true;
	}
	mutex_unlock(&tx_device->mutex);
}
EXPORT_SYMBOL_GPL(video_tx_request_polling);

/**
 * video_tx_cancel_polling - cancel the polling thread
 *
 * @tx_device: pointer to the video tx device
 *
 * This function will be called by the video tx client when it is no longer
 * interested in reporting hotplug events through polling. After this function
 * returns it is guaranteed that the polling thread will not call detect()
 * anymore in the client.
 *
 * Cannot be called from an atomic context.
 */
void video_tx_cancel_polling(struct video_tx_device *tx_device)
{
	mutex_lock(&tx_device->mutex);
	tx_device->polling_enabled = false;
	cancel_delayed_work_sync(&tx_device->polling_dwork);
	mutex_unlock(&tx_device->mutex);
}
EXPORT_SYMBOL(video_tx_cancel_polling);

/**
 * video_tx_register_device - register a video tx with the framework
 *
 * @dev: pointer to the video transmitter device structure
 * @driver: pointer to the driver structure that contains all the ops
 *
 * This function will register a video transmitter client with the video_tx
 * framework.
 * It will return a pointer to the newly allocated video_tx_device structure or
 * NULL in case of an error.
 *
 * The video tx driver needs to call this function before using any of the
 * other facilities of the framework.
 *
 */
struct video_tx_device *video_tx_register_device(struct device *dev,
					struct video_tx_driver *driver)
{
	struct video_tx_device *tx_device;

	tx_device = kzalloc(sizeof(struct video_tx_device), GFP_KERNEL);
	if (!tx_device)
		return NULL;

	INIT_LIST_HEAD(&tx_device->list);
	tx_device->dev = dev;
	tx_device->driver = driver;
	mutex_init(&tx_device->mutex);
	BLOCKING_INIT_NOTIFIER_HEAD(&tx_device->notifier);
	INIT_DELAYED_WORK(&tx_device->polling_dwork, video_tx_polling_work);
	tx_device->polling_enabled = false;

	/* Add it to the global list of registered drivers */
	mutex_lock(&client_mutex);
	list_add_tail(&tx_device->list, &video_tx_list);
	mutex_unlock(&client_mutex);

	return tx_device;
}
EXPORT_SYMBOL_GPL(video_tx_register_device);

/**
 * video_tx_unregister_device - unregister a video tx with the framework
 *
 * @tx_device: pointer to the video tx device
 *
 */
void video_tx_unregister_device(struct video_tx_device *tx_device)
{

	mutex_lock(&client_mutex);
	list_del(&tx_device->list);
	mutex_unlock(&client_mutex);

	mutex_lock(&tx_device->mutex);
	if (tx_device->polling_enabled) {
		tx_device->polling_enabled = false;
		cancel_delayed_work_sync(&tx_device->polling_dwork);
	}
	mutex_unlock(&tx_device->mutex);

	tx_device->driver = NULL;
	tx_device->dev = NULL;
	tx_device->private = NULL;

	kfree(tx_device);
}
EXPORT_SYMBOL_GPL(video_tx_unregister_device);

void video_tx_set_drvdata(struct video_tx_device *tx_device, void *data)
{
	tx_device->private = data;
}
EXPORT_SYMBOL_GPL(video_tx_set_drvdata);

void *video_tx_get_drvdata(struct video_tx_device *tx_device)
{
	return tx_device->private;
}
EXPORT_SYMBOL_GPL(video_tx_get_drvdata);

/* Module boilerplate */
static int __init video_tx_init(void)
{
	video_tx_wq = create_singlethread_workqueue("video-tx-polling");
	if (!video_tx_wq) {
		pr_err("video-tx: could not initialize work queue\n");
		return -ENOMEM;
	}

	pr_info("video-tx: initialized (polling_period_ms = %hu)\n",
		polling_period_ms);

	return 0;
}
module_init(video_tx_init);

static void __exit video_tx_exit(void)
{
	if (video_tx_wq)
		destroy_workqueue(video_tx_wq);

	WARN_ON(!list_empty(&video_tx_list));
}
module_exit(video_tx_exit);

module_param(polling_period_ms, ushort, 0444);
MODULE_PARM_DESC(polling_period_ms, "Period in ms of the hotplug polling thread");

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Video-TX Framework");
