/*
 * Copyright (C) 2015 Intel Mobile Communications GmbH
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Notes:
 * Dec 17 2014: IMC: Add initial xgold platform configuration
 *
 */

#ifndef __MALI_KBASE_PLATFORM_XGOLD__
#define __MALI_KBASE_PLATFORM_XGOLD__

#define MALI_PLF_NAME "Mali Midgard Platform"
#define mali_err(fmt, arg...)	pr_err(MALI_PLF_NAME" [ERROR]: " fmt, ##arg)
#define mali_info(fmt, arg...)	pr_info(MALI_PLF_NAME": " fmt, ##arg)
#define mali_warn(fmt, arg...)	pr_warn(MALI_PLF_NAME" [W]: " fmt, ##arg)
#define mali_dbg(fmt, arg...)	pr_debug(MALI_PLF_NAME" [D]: " fmt, ##arg)
#define mali_test(fmt, arg...)	pr_err(MALI_PLF_NAME" [TEST]: " fmt, ##arg)


#undef GPU_USE_ULTRA_HIGH_PERF /* Enable to use ultra_high_perf mode */
#if defined(GPU_USE_ULTRA_HIGH_PERF)
#define GPU_NUM_PM_STATES 5
#else
#define GPU_NUM_PM_STATES 4
#endif /* defined(GPU_USE_ULTRA_HIGH_PERF) */
#define GPU_MIN_PM_STATE 1
#define GPU_MAX_PM_STATE (GPU_NUM_PM_STATES - 1)
#define GPU_INITIAL_PM_STATE GPU_MAX_PM_STATE

/* PM states index */
#define MALI_PLF_PM_STATE_D3	0
#define MALI_PLF_PM_STATE_D0	GPU_MAX_PM_STATE
#define MALI_PLF_PM_STATE_D1	(MALI_PLF_PM_STATE_D0-1)


struct xgold_platform_context {
	struct device_pm_platdata *pm_platdata;
	struct platform_device_pm_state *pm_states[GPU_NUM_PM_STATES];
	unsigned int curr_pm_state;
	unsigned int resume_pm_state;
	bool restore_gpu_qos;
#ifdef CONFIG_MALI_MIDGARD_DVFS
	struct kbase_device *kbdev;
	unsigned int utilization;
	struct workqueue_struct *mali_dvfs_wq;
	struct work_struct dvfs_work;
	spinlock_t pm_lock;
	bool dvfs_off;
#endif
#ifdef CONFIG_MALI_MIDGARD_XGOLD_THERMAL
	struct thermal_cooling_device *cooling_dev;
#endif
	struct mutex pm_lock_mutex;
};
#ifdef CONFIG_MALI_MIDGARD_DVFS
#define map_to_xgold_platform_context(_p_, _m_) \
		container_of(_p_, struct xgold_platform_context, _m_)
#endif
mali_error kbase_platform_init(struct kbase_device *kbdev);
int kbase_platform_xgold_pm_control(struct kbase_device *kbdev,
							int req_pm_state);
void kbase_platform_term(struct kbase_device *kbdev);

#endif /*__MALI_KBASE_PLATFORM_XGOLD__*/
