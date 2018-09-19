/*
 ****************************************************************
 *
 *  Component: Android HAL libgralloc
 *
 *  Copyright (C) 2013, 2014 Intel Mobile Communications GmbH
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
 *
 ****************************************************************
 */

#include <sys/mman.h>

#include <dlfcn.h>

#include <cutils/log.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <linux/ion.h>
#include <linux/xgold_ion.h>
#include "ion-hal.h"

static int ion_ioctl(int fd, int req, void *arg)
{
        int ret = ioctl(fd, req, arg);
        if (ret < 0) {
                ALOGE("%s %d failed with code %d: %s\n", __FUNCTION__, req,
                       ret, strerror(errno));
                return -errno;
        }
        return ret;
}

int ion_get_param(int fdion, int bufid, unsigned int *phys, size_t *len)
{
	struct xgold_ion_get_params_data xgold_data;
	struct ion_custom_data custom_data;
	int err=0;

	xgold_data.handle = bufid;

	custom_data.cmd = XGOLD_ION_GET_PARAM;
	custom_data.arg = (unsigned long)(&xgold_data);

	err = ion_ioctl(fdion, ION_IOC_CUSTOM, &custom_data);
	if(err){
		ALOGE("%s ION_IOC_CUSTOM (GET_PARAM) failed\n", __FUNCTION__);
		*phys = 0;
		return -1;
	}
	*phys = (unsigned int) xgold_data.addr;
	*len = xgold_data.size;
	return err;
}

int ion_alloc_secure(int fdion, size_t *len, unsigned int *phys)
{
	struct xgold_ion_get_params_data xgold_data;
	struct ion_custom_data custom_data;
	int err = -127;

  if(phys != NULL && len != NULL) {
    xgold_data.handle = 0;
    xgold_data.size   = *len;

    custom_data.cmd = XGOLD_ION_ALLOC_SECURE;
    custom_data.arg = (unsigned long)(&xgold_data);

    err = ion_ioctl(fdion, ION_IOC_CUSTOM, &custom_data);

    if(err) {
      ALOGE("%s ION_IOC_CUSTOM (ALLOC) failed\n", __FUNCTION__);
      *phys = 0;

      err = -1;
    } else {
      *phys = (unsigned int) xgold_data.addr;
      *len  =       (size_t) xgold_data.size;

      err = 0;
    }
  }

  ALOGV("ion_alloc_secure(fd %d, len %u, phys 0x%08x) = %d",
    fdion, *len, *phys, err);

	return err;
}

int ion_free_secure(int fdion, size_t len, unsigned int phys)
{
	struct xgold_ion_get_params_data xgold_data;
	struct ion_custom_data custom_data;
	int err = -127;

  if(phys != 0 && len != 0) {
    xgold_data.handle = 0;
    xgold_data.size   = len;
    xgold_data.addr   = phys;

    custom_data.cmd = XGOLD_ION_FREE_SECURE;
    custom_data.arg = (unsigned long)(&xgold_data);

    err = ion_ioctl(fdion, ION_IOC_CUSTOM, &custom_data);

    if(err) {
      ALOGE("%s ION_IOC_CUSTOM (FREE) failed\n", __FUNCTION__);

      err = -1;
    } else {
      err = 0;
    }
  }

  ALOGV("ion_free_secure(fd %d, len %u, phys 0x%08x) = %d",
    fdion, len, phys, err);

	return err;
}
