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



#include <linux/types.h>
#include <linux/slab.h>
#include <video/adf.h>
#include <uapi/video/malidp_adf.h>

#include "malidp_adf.h"

/* Private, driver-specific overlay structure */
struct malidp_overlay {
	struct adf_overlay_engine eng;
	struct adf_overlay_engine_ops ops;
	const struct malidp_layer_hw_info *hw_info;
};

#define to_malidp_overlay(adf_eng) container_of(adf_eng, \
		struct malidp_overlay, eng)

const struct malidp_layer_hw_info *malidp_adf_ovr_get_hw_layer(struct adf_overlay_engine *eng)
{
	struct malidp_overlay *malidp_ovl = to_malidp_overlay(eng);

	return malidp_ovl->hw_info;
}

static int malidp_ovr_custom_data(struct adf_obj *obj, void *vdata, size_t *size)
{
	struct adf_overlay_engine *ovl = adf_obj_to_overlay_engine(obj);
	struct malidp_overlay *malidp_ovl = to_malidp_overlay(ovl);
	struct malidp_adf_overlay_custom_data *data = vdata;

	memset(data, 0, sizeof(*data));

	data->supports_scaling = malidp_ovl->hw_info->features &
		MALIDP_LAYER_FEATURE_SCALING;
	data->features = malidp_ovl->hw_info->features;
	data->n_supported_layers = malidp_ovl->hw_info->n_supported_layers;

	*size = sizeof(*data);

	return 0;
}

static const struct adf_overlay_engine_ops malidp_ovr_ops = {
	.base = {
		.custom_data = malidp_ovr_custom_data,
	},
};

static int malidp_ovr_hw_info_to_adf(struct malidp_overlay *malidp_ovl,
				     const struct malidp_layer_hw_info *hw_info)
{
	size_t *ptr_n_supported_formats = (size_t *)&malidp_ovl->ops.n_supported_formats;
	struct adf_overlay_engine_ops *ops = (struct adf_overlay_engine_ops *)&malidp_ovl->ops;

	/* Assign ops */
	memcpy(ops, &malidp_ovr_ops, sizeof(*ops));
	*ptr_n_supported_formats = hw_info->n_supported_formats;
	malidp_ovl->ops.supported_formats = hw_info->supported_formats;

	malidp_ovl->hw_info = hw_info;

	return 0;
}

/*
 * Add a layer to the adf_device. Allocates an overlay engine with the
 * parameters of the specified type, and initialises it on the given adf
 * device.
 *
 * @dev: The adf device which the overlay engine will belong to
 * @info: The HW description of the layer to add
 *
 * Returns the adf object id if successful, an error code (<0) on error.
 */
static int malidp_add_layer(struct malidp_device *dp_dev,
		const struct malidp_layer_hw_info *info)
{
	struct adf_device *dev = &dp_dev->adf_dev;
	struct malidp_overlay *malidp_ovl;
	int ret;

	malidp_ovl = kzalloc(sizeof(struct malidp_overlay), GFP_KERNEL);
	if (!malidp_ovl)
		return -ENOMEM;

	/* Convert HW layer information into ADF information */
	ret = malidp_ovr_hw_info_to_adf(malidp_ovl, info);
	if (ret < 0)
		goto fail;

	ret = adf_overlay_engine_init(&malidp_ovl->eng, dev, &malidp_ovl->ops,
			"%s", malidp_ovl->hw_info->name);
	if (ret)
		goto fail;

	return malidp_ovl->eng.base.id;

fail:
	kfree(malidp_ovl);

	return ret;
}

int malidp_adf_ovr_add_layers(struct malidp_device *dp_dev,
		const struct malidp_layer_hw_info *layers, int n_layers)
{
	int i, ret = 0;

	for (i = 0; i < n_layers; i++) {
		ret = malidp_add_layer(dp_dev, &layers[i]);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int malidp_eng_destroy_cb(int id, void *p, void *opaque)
{
	struct adf_overlay_engine *eng = (struct adf_overlay_engine *)p;
	struct malidp_overlay *ovl = to_malidp_overlay(eng);

	dev_dbg(&eng->base.dev, "Destroying overlay engine %s\n", eng->base.name);

	adf_overlay_engine_destroy(eng);
	kfree(ovl);

	return 0;
}

int malidp_adf_ovr_destroy_layers(struct malidp_device *dp_dev)
{
	struct adf_device *adf_dev = &dp_dev->adf_dev;

	return idr_for_each(&adf_dev->overlay_engines, malidp_eng_destroy_cb,
			NULL);
}

