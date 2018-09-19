/*
 *
 * (C) COPYRIGHT 2013-2015 ARM Limited. All rights reserved.
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



#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <video/video_tx.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/suspend.h>

#include "malidp_adf.h"
#include "malidp_drv.h"
#include "malidp_hw.h"
#include "malidp_hw_types.h"
#include "malidp_iommu.h"
#include "malidp_sysfs.h"
#include "malidp_of_graph.h"
#define MALIDP_NAME "mali-dp"
/* Epoch should be limited to 8 bits, and major and minor to 12 bits */
#define VERSION_EPOCH 1
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

struct malidp_devtype {
	enum malidp_hw_product product;
};

static const struct malidp_devtype malidp_devdata[] = {
	[MALI_DP500] = {
		.product = MALI_DP500,
	},
	[MALI_DP550] = {
		.product = MALI_DP550,
	},
};

static const struct of_device_id malidp_dt_ids[] = {
	{ .compatible = "arm,mali-dp500", .data = &malidp_devdata[MALI_DP500], },
	{ .compatible = "arm,mali-dp550", .data = &malidp_devdata[MALI_DP550], },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, malidp_dt_ids);

struct video_tx_device *malidp_find_video_tx(struct malidp_device *dev,
		struct device_node *nproot)
{
	struct device_node *local_endpoint;
	struct device_node *tx_node;
	struct video_tx_device *tx = NULL;

	local_endpoint = of_graph_get_next_endpoint(nproot, NULL);
	if (!local_endpoint) {
		dev_err(dev->device, "Couldn't find local endpoint\n");
		return NULL;
	}

	tx_node = of_graph_get_remote_port_parent(local_endpoint);
	if (tx_node) {
		tx = of_find_video_tx_by_node(tx_node);
		of_node_put(tx_node);
	} else {
		dev_err(dev->device, "Couldn't find transmitter by endpoint\n");
	}
	of_node_put(local_endpoint);

	return tx;
}

#if defined(CONFIG_PM_SLEEP) || defined(CONFIG_PM_RUNTIME)
static struct malidp_device *to_malidp_dev(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return platform_get_drvdata(pdev);
}
#endif /* CONFIG_PM_SLEEP | CONFIG_PM_RUNTIME */

#ifdef CONFIG_PM_SLEEP
static int malidp_sys_pm_suspend(struct device *dev)
{
	struct malidp_device *dp_dev = to_malidp_dev(dev);

	dev_info(dev, "%s\n", __func__);

	if (dp_dev->current_dpms == DRM_MODE_DPMS_ON)
		malidp_hw_display_switch(dp_dev->hw_dev, false);

	return 0;
}

static int malidp_sys_pm_suspend_late(struct device *dev)
{
	struct malidp_device *dp_dev = to_malidp_dev(dev);

	dev_info(dev, "%s\n", __func__);
	if (!pm_runtime_status_suspended(dev)) {
		malidp_hw_runtime_suspend(dp_dev->hw_dev);
		pm_runtime_set_suspended(dev);
	}
	return 0;
}

static int malidp_sys_pm_resume_early(struct device *dev)
{
	struct malidp_device *dp_dev = to_malidp_dev(dev);

	dev_info(dev, "%s\n", __func__);
	malidp_hw_runtime_resume(dp_dev->hw_dev);
	pm_runtime_set_active(dev);
	return 0;
}

static void malidp_sys_pm_complete(struct device *dev)
{
	struct malidp_device *dp_dev = to_malidp_dev(dev);

	dev_info(dev, "%s\n", __func__);

	if (dp_dev->current_dpms == DRM_MODE_DPMS_ON) {
		if (dp_dev->adf_dev.onscreen != NULL) {
			struct malidp_driver_state *state =
				dp_dev->adf_dev.onscreen->state;
			malidp_adf_cleanup_signaled_mw(state);
			malidp_hw_commit(dp_dev->hw_dev, &state->hw_state);
		}
		malidp_hw_display_switch(dp_dev->hw_dev, true);
	}
	atomic_set(&dp_dev->suspending, 0);
}

static int malidp_pm_notifier(struct notifier_block *notifier,
		unsigned long event, void *data)
{
	struct malidp_device *dev = container_of(notifier,
						struct malidp_device,
						dp_pm_nb);
	struct malidp_driver_state *state;

	switch (event) {
	case PM_SUSPEND_PREPARE:
		dev_info(dev->device, "%s:PM_SUSPEND_PREPARE\n", __func__);

		atomic_set(&dev->suspending, 1);

		mutex_lock(&dev->adf_dev.client_lock);
		flush_kthread_worker(&dev->adf_dev.post_worker);
		if (dev->adf_dev.onscreen != NULL) {
			state = dev->adf_dev.onscreen->state;
			malidp_adf_waiting_for_mw(state);
		}
		mutex_unlock(&dev->adf_dev.client_lock);

		break;
	case PM_POST_SUSPEND:
		dev_info(dev->device, "%s:PM_POST_SUSPEND\n", __func__);
		atomic_set(&dev->suspending, 0);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}
#endif /* CONFIG_PM_SLEEP */

static int malidp_remove(struct platform_device *pdev)
{
	struct malidp_device *dev = platform_get_drvdata(pdev);

	dev_dbg(&pdev->dev, "removing module");

#ifdef CONFIG_PM_SLEEP
	unregister_pm_notifier(&dev->dp_pm_nb);
#endif /* CONFIG_PM_SLEEP */

	pm_runtime_disable(&pdev->dev);

	debugfs_remove_recursive(dev->dbg_folder);

	malidp_sysfs_destroy(dev);
	malidp_adf_destroy(dev);
	if (dev->iommu_domain)
		malidp_iommu_exit(dev->iommu_domain);
	if (!pm_runtime_status_suspended(&pdev->dev)) {
		malidp_hw_runtime_suspend(dev->hw_dev);
		pm_runtime_set_suspended(&pdev->dev);
	}

	malidp_hw_exit(dev->hw_dev);

	return 0;
}

static int malidp_debugfs_init(struct malidp_device *dev)
{
	char dbg_folder_name[32];

	if (!debugfs_initialized())
		return 0;

	snprintf(dbg_folder_name, 32, "%s%d", dev->name, dev->id);
	dev->dbg_folder = debugfs_create_dir(dbg_folder_name, NULL);
	if (IS_ERR_OR_NULL(dev->dbg_folder))
		return -EINVAL;
	return malidp_hw_debugfs_init(dev->hw_dev, dev->dbg_folder);
}

static int malidp_probe(struct platform_device *pdev)
{
	const struct of_device_id *of_id =
		of_match_device(of_match_ptr(malidp_dt_ids), &pdev->dev);
	struct device_node *np = pdev->dev.of_node;
	struct malidp_device *dev;
	struct malidp_hw_description hw_desc;
	struct malidp_hw_pdata pdata;
	struct video_tx_device *tx;
	const struct malidp_devtype *malidp_dev;
	int ret;

	if (!of_id || !np) {
		dev_err(&pdev->dev, "only device tree set up supported\n");
		return -ENODEV;
	}
	malidp_dev = of_id->data;

	dev = devm_kzalloc(&pdev->dev,
			     sizeof(struct malidp_device),
			     GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->device = &pdev->dev;

	dev_dbg(&pdev->dev, "name: %s\n", of_id->name);
	memset(&pdata, 0, sizeof(struct malidp_hw_pdata));
	ret = malidp_hw_get_resources(pdev, np, &pdata);
	if (ret < 0)
		return ret;

	dev->id = pdata.dp_id;

	malidp_hw_enumerate(&hw_desc, malidp_dev->product, &pdata);

	dev->hw_dev = malidp_hw_init(pdev, &hw_desc);
	if (IS_ERR(dev->hw_dev)) {
		dev_err(&pdev->dev, "failed to initialize the HW");
		return PTR_ERR(dev->hw_dev);
	}

	dev->core_id = malidp_hw_get_core_id(dev->hw_dev);

	snprintf(dev->name, ADF_NAME_LEN, "malidp");

	dev->iommu_domain = malidp_iommu_init(&pdev->dev, pdev->dev.bus);
	if (!dev->iommu_domain)
		dev_info(&pdev->dev, "Continuing without IOMMU support\n");

	tx = malidp_find_video_tx(dev, np);
	if (!tx) {
		dev_err(&pdev->dev, "deferring probe: failed to find video tx\n");
		ret = -EPROBE_DEFER;
		goto err_hw;
	}

	ret = malidp_adf_init(dev, &hw_desc, tx);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize ADF device");
		goto err_hw;
	}

	ret = malidp_sysfs_init(dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize sysfs interface");
		goto err_adf;
	}

	ret = malidp_debugfs_init(dev);
	if (ret) {
		dev_err(&pdev->dev, "failed initializing debugfs\n");
		goto err_sysfs;
	}

	pm_runtime_set_suspended(&pdev->dev);
	pm_runtime_enable(&pdev->dev);
	if (!pm_runtime_enabled(&pdev->dev)) {
		dev_info(&pdev->dev, "Continuing without Runtime PM support\n");
		malidp_hw_runtime_resume(dev->hw_dev);
		pm_runtime_set_active(&pdev->dev);
	}

#ifdef CONFIG_PM_SLEEP
	dev->dp_pm_nb.notifier_call = malidp_pm_notifier;
	ret = register_pm_notifier(&dev->dp_pm_nb);
	if (ret) {
		dev_err(&pdev->dev, "could not register pm notifier\n");
		goto err_late;
	}
#endif

	platform_set_drvdata(pdev, dev);

	return 0;

#ifdef CONFIG_PM_SLEEP
err_late:
	pm_runtime_disable(&pdev->dev);
	if (!pm_runtime_status_suspended(&pdev->dev)) {
		malidp_hw_runtime_suspend(dev->hw_dev);
		pm_runtime_set_suspended(&pdev->dev);
	}

	debugfs_remove_recursive(dev->dbg_folder);
#endif
err_sysfs:
	malidp_sysfs_destroy(dev);
err_adf:
	malidp_adf_destroy(dev);
err_hw:
	if (dev->iommu_domain)
		malidp_iommu_exit(dev->iommu_domain);
	malidp_hw_exit(dev->hw_dev);
	return ret;
}

#ifdef CONFIG_PM_RUNTIME
static int malidp_rt_pm_suspend(struct device *dev)
{
	struct malidp_device *dp_dev = to_malidp_dev(dev);

	dev_info(dev, "%s\n", __func__);

	malidp_hw_runtime_suspend(dp_dev->hw_dev);
	return 0;
}

static int malidp_rt_pm_resume(struct device *dev)
{
	struct malidp_device *dp_dev = to_malidp_dev(dev);

	dev_info(dev, "%s\n", __func__);
	malidp_hw_runtime_resume(dp_dev->hw_dev);
	malidp_adf_runtime_resume(dp_dev);
	return 0;
}
#endif /* ! CONFIG_PM_RUNTIME */

static const struct dev_pm_ops malidp_pm_ops = {
	SET_RUNTIME_PM_OPS(malidp_rt_pm_suspend,
			   malidp_rt_pm_resume,
			   NULL)
#ifdef CONFIG_PM_SLEEP
	.suspend = malidp_sys_pm_suspend,
	.suspend_late = malidp_sys_pm_suspend_late,
	.resume_early = malidp_sys_pm_resume_early,
	.complete = malidp_sys_pm_complete,
#endif
};

static struct platform_driver malidp_driver = {
	.probe  = malidp_probe,
	.remove = malidp_remove,
	.driver = {
		.name   = MALIDP_NAME,
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(malidp_dt_ids),
		.pm = &malidp_pm_ops,
	},
};

module_platform_driver(malidp_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mali-DP product driver");
MODULE_VERSION(__stringify(VERSION_EPOCH) ":"
	       __stringify(VERSION_MAJOR) "."
	       __stringify(VERSION_MINOR)
#ifdef DEBUG
	       "-debug"
#endif
);
