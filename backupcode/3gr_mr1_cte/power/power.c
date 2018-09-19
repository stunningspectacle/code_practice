/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_NDEBUG 0

#define LOG_TAG "IntelPowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

/* FIXME: diff board revision */
#ifdef BOARD_REV_3G
#define SYSFS_TS_SUSPEND_FILE "/sys/power/power_HAL_suspend/1-0014/power_HAL_suspend"
#define SYSFS_DISPLAY_SUSPEND_FILE "/sys/power/power_HAL_suspend/e1000000.dcc/power_HAL_suspend"

#elif defined BOARD_REV_LTE

#define SYSFS_TS_SUSPEND_FILE "/sys/power/power_HAL_suspend/3-0014/power_HAL_suspend"
#define SYSFS_DISPLAY_SUSPEND_FILE "/sys/power/power_HAL_suspend/eb100000.dcc/power_HAL_suspend"

#endif


static void sysfs_write(const char *path, char *s)
{
	char buf[80];
	int len;
	int fd = open(path, O_WRONLY);

	if (fd < 0) {
		strerror_r(errno, buf, sizeof(buf));
		ALOGE("Error opening %s: %s\n", path, buf);
		return;
	}

	len = write(fd, s, strlen(s));
	if (len < 0) {
		strerror_r(errno, buf, sizeof(buf));
		ALOGE("Error writing to %s: %s\n", path, buf);
	}

	close(fd);
}

static void power_init(struct power_module *module)
{
    ALOGI("%s", __func__);
}

static void power_set_interactive(struct power_module *module, int on)
{
#ifdef EARLY_SUSPEND_SUPPORT
    ALOGI("%s %s\n", __func__, (on ? "ON" : "OFF"));
    sysfs_write(SYSFS_TS_SUSPEND_FILE, on ? "0" : "1");
    sysfs_write(SYSFS_DISPLAY_SUSPEND_FILE, on ? "0" : "1");

    ALOGI("%s done\n", __func__);
#endif
}

static void power_hint(struct power_module *module, power_hint_t hint,
                       void *data) {
    switch (hint) {
    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Intel Power HAL",
        .author = "The Android Open Source Project",
        .methods = &power_module_methods,
    },

    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
};
