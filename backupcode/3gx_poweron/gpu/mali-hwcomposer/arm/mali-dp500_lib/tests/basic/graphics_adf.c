/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/cdefs.h>
#include <sys/mman.h>

#include <adf/adf.h>

#include <ion/ion.h>
#include "graphics.h"

struct adf_surface_pdata {
    GRSurface base;
    ion_user_handle_t h_ion;
    int fd;
    __u32 offset;
    __u32 pitch;
};

struct adf_pdata {
    minui_backend base;
    int intf_fd;
    adf_id_t eng_id;
    __u32 format;

    unsigned int current_surface;
    unsigned int n_surfaces;
    struct adf_surface_pdata surfaces[2];
};

static gr_surface adf_flip(struct minui_backend *backend);
static void adf_blank(struct minui_backend *backend, bool blank);

static int ion_fd = -1;
static int cust_simple_buffer_alloc(__u32 w, __u32 h, __u32 format,
				__u32 *offset, __u32 *pitch,
				ion_user_handle_t *ph_ion)
{
	int share_fd = -1;
	int size = 0;
	int res = -1;

	size = w*h*4;
	ion_fd = ion_open();
	if (ion_fd < 0)
		return -1;

	res = ion_alloc(ion_fd, size, 8, ION_HEAP_TYPE_DMA_MASK, 0, ph_ion);
	if (res < 0)
		return -1;

	res = ion_share(ion_fd, *ph_ion, &share_fd);
	if ((res < 0) || (share_fd < 0))
		return -1;


	*offset = 0;
	/* pitch in bytes */
	*pitch = w * 4;	
	
	return share_fd;
}

static int adf_surface_init(struct adf_pdata *pdata,
        struct drm_mode_modeinfo *mode, struct adf_surface_pdata *surf)
{
    memset(surf, 0, sizeof(*surf));

#if 0
    surf->fd = adf_interface_simple_buffer_alloc(pdata->intf_fd, mode->hdisplay,
            mode->vdisplay, pdata->format, &surf->offset, &surf->pitch);
#endif
    surf->fd = cust_simple_buffer_alloc(mode->hdisplay,
					mode->vdisplay,
					pdata->format,
					&surf->offset,
					&surf->pitch,
					&surf->h_ion);
    if (surf->fd < 0)
        return surf->fd;

    surf->base.width = mode->hdisplay;
    surf->base.height = mode->vdisplay;
    surf->base.row_bytes = surf->pitch;
    surf->base.pixel_bytes = (pdata->format == DRM_FORMAT_RGB565) ? 2 : 4;

    surf->base.data = mmap(NULL, surf->pitch * surf->base.height, PROT_WRITE,
            MAP_SHARED, surf->fd, surf->offset);
    if (surf->base.data == MAP_FAILED) {
        close(surf->fd);
        return -errno;
    }

    return 0;
}

static int adf_interface_init(struct adf_pdata *pdata)
{
    struct adf_interface_data intf_data;
    int ret = 0;
    int err, i;
    struct drm_mode_modeinfo *mode = NULL;

    err = adf_get_interface_data(pdata->intf_fd, &intf_data);
    if (err < 0)
        return err;

    if ((0 == intf_data.current_mode.hdisplay) || (0 == intf_data.current_mode.vdisplay)) {
      fprintf(stderr, "as expected NO current_mode!!\n");
      if (intf_data.n_available_modes) {
        /* Look for preferred mode */
        for (i = 0; i < intf_data.n_available_modes; i++) {
          mode = &intf_data.available_modes[i];
          if (mode->type & DRM_MODE_TYPE_PREFERRED) {
            fprintf(stderr, "DRM_MODE_TYPE_PREFERRED found!!\n");
            break;
          }
        }
      }

      adf_interface_blank(pdata->intf_fd, DRM_MODE_DPMS_OFF);
      adf_interface_set_mode(pdata->intf_fd, mode);
      adf_interface_blank(pdata->intf_fd, DRM_MODE_DPMS_ON);
      err = adf_get_interface_data(pdata->intf_fd, &intf_data);
      fprintf(stderr, "New: %dx%d\n", intf_data.current_mode.hdisplay, intf_data.current_mode.vdisplay);
    } else {
      fprintf(stderr, "currentmode exists!!\n");
      fprintf(stderr, "default: %dx%d\n", intf_data.current_mode.hdisplay, intf_data.current_mode.vdisplay);
    }

    err = adf_surface_init(pdata, &intf_data.current_mode, &pdata->surfaces[0]);
    if (err < 0) {

        fprintf(stderr, "allocating surface 0 failed: %s\n", strerror(-err));
        ret = err;
        goto done;
    }

    err = adf_surface_init(pdata, &intf_data.current_mode,
            &pdata->surfaces[1]);
    if (err < 0) {
        fprintf(stderr, "allocating surface 1 failed: %s\n", strerror(-err));
        memset(&pdata->surfaces[1], 0, sizeof(pdata->surfaces[1]));
        pdata->n_surfaces = 1;
    } else {
        pdata->n_surfaces = 2;
    }

done:
    adf_free_interface_data(&intf_data);
    return ret;
}

static int adf_device_init(struct adf_pdata *pdata, struct adf_device *dev)
{
    adf_id_t intf_id;
    int intf_fd;
    int err;

    err = adf_find_simple_post_configuration(dev, &pdata->format, 1, &intf_id,
            &pdata->eng_id);
    if (err < 0)
        return err;

    err = adf_device_attach(dev, pdata->eng_id, intf_id);
    if (err < 0 && err != -EALREADY)
        return err;

    pdata->intf_fd = adf_interface_open(dev, intf_id, O_RDWR);
    if (pdata->intf_fd < 0)
        return pdata->intf_fd;

    err = adf_interface_init(pdata);
    if (err < 0) {
        close(pdata->intf_fd);
        pdata->intf_fd = -1;
    }

    return err;
}

static gr_surface adf_init(minui_backend *backend)
{
    struct adf_pdata *pdata = (struct adf_pdata *)backend;
    adf_id_t *dev_ids = NULL;
    ssize_t n_dev_ids, i;
    gr_surface ret;

#if defined(RECOVERY_BGRA)
    pdata->format = DRM_FORMAT_BGRA8888;
#elif defined(RECOVERY_XBGR)
    pdata->format = DRM_FORMAT_XBGR8888;
#else
    pdata->format = DRM_FORMAT_RGB565;
#endif

    n_dev_ids = adf_devices(&dev_ids);
    if (n_dev_ids == 0) {
        return NULL;
    } else if (n_dev_ids < 0) {
        fprintf(stderr, "enumerating adf devices failed: %s\n",
                strerror(-n_dev_ids));
        return NULL;
    }

    pdata->intf_fd = -1;

    for (i = 0; i < n_dev_ids && pdata->intf_fd < 0; i++) {
        struct adf_device dev;

        int err = adf_device_open(dev_ids[i], O_RDWR, &dev);
        if (err < 0) {
            fprintf(stderr, "opening adf device %u failed: %s\n", dev_ids[i],
                    strerror(-err));
            continue;
        }

        err = adf_device_init(pdata, &dev);
        if (err < 0)
            fprintf(stderr, "initializing adf device %u failed: %s\n",
                    dev_ids[i], strerror(-err));

        adf_device_close(&dev);
    }

    free(dev_ids);

    if (pdata->intf_fd < 0)
        return NULL;

    ret = adf_flip(backend);

    adf_blank(backend, true);
    adf_blank(backend, false);

    return ret;
}

static gr_surface adf_flip(struct minui_backend *backend)
{
    struct adf_pdata *pdata = (struct adf_pdata *)backend;
    struct adf_surface_pdata *surf = &pdata->surfaces[pdata->current_surface];

    int fence_fd = adf_interface_simple_post(pdata->intf_fd, pdata->eng_id,
            surf->base.width, surf->base.height, pdata->format, surf->fd,
            surf->offset, surf->pitch, -1);
    if (fence_fd >= 0)
        close(fence_fd);

    pdata->current_surface = (pdata->current_surface + 1) % pdata->n_surfaces;
    return &pdata->surfaces[pdata->current_surface].base;
}

static void adf_blank(struct minui_backend *backend, bool blank)
{
    struct adf_pdata *pdata = (struct adf_pdata *)backend;
    adf_interface_blank(pdata->intf_fd,
            blank ? DRM_MODE_DPMS_OFF : DRM_MODE_DPMS_ON);
}

static void adf_surface_destroy(struct adf_surface_pdata *surf)
{
    munmap(surf->base.data, surf->pitch * surf->base.height);
    close(surf->fd);
    ion_free(ion_fd, surf->h_ion);
}

static void adf_exit(struct minui_backend *backend)
{
    struct adf_pdata *pdata = (struct adf_pdata *)backend;
    unsigned int i;

    for (i = 0; i < pdata->n_surfaces; i++)
        adf_surface_destroy(&pdata->surfaces[i]);
    close(ion_fd);
    if (pdata->intf_fd >= 0)
        close(pdata->intf_fd);
    free(pdata);
}

minui_backend *open_adf()
{
    struct adf_pdata *pdata = calloc(1, sizeof(*pdata));
    if (!pdata) {
        perror("allocating adf backend failed");
        return NULL;
    }

    pdata->base.init = adf_init;
    pdata->base.flip = adf_flip;
    pdata->base.blank = adf_blank;
    pdata->base.exit = adf_exit;
    return &pdata->base;
}
