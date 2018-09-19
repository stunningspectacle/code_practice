/*
 *****************************************************************************
 * Copyright (C) 2013 Intel Mobile Communications GmbH
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
 ******************************************************************************
 */


#ifndef __DCC_HAL_H_
#define __DCC_HAL_H_

#include <video/xgold-dcc.h>

/* =================== DCC hal attributes: START ================================= */
#define DCC_HAL_ATTR_DISPLAY_INFO	1
#define DCC_HAL_ATTR_VSYNC	2
#define VSYNC_ON	1
#define VSYNC_OFF	0
struct dcc_hal_attr_t {
	/* TODO: multi display support */
	struct dcc_display_cfg_t display_info;
	unsigned int vsync;
	/* TODO: extend if needed */
};
/* =================== DCC hal attributes: END ================================= */

#ifdef __cplusplus
extern "C" {
#endif

#define HWC_OVERLAY_NUM 2
#define HWC_OVERLAY_INVALID HWC_OVERLAY_NUM
/* TODO: support dynamical layer count */
#define HWC_MAX_LAYER_NUM 10

int  dcc_hal_init(void);
void dcc_hal_deinit(void);
int  dcc_hal_get_attr(int attr_type, struct dcc_hal_attr_t *attr);
int  dcc_hal_set_attr(int attr_type, struct dcc_hal_attr_t *attr);
int dcc_hal_compose(hwc_layer_1_t* fb_layer, hwc_layer_1_t** ov_layers,
        size_t ov_count, int fb_enabled, int *retireFenceFd);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Map to the framework debug prints */
#include <stdarg.h>
#include <cutils/log.h>
#define dcc_hal_print(...)	ALOGV(__VA_ARGS__)
#define dcc_hal_dbg(...)	ALOGV(__VA_ARGS__)
#define dcc_hal_warn(...)	ALOGW(__VA_ARGS__)
#define dcc_hal_err(...)	ALOGE(__VA_ARGS__)

#endif /* __DCC_HAL_H_ */
