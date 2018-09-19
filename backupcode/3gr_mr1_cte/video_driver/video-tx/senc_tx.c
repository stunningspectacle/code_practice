/*
 *
 * (C) COPYRIGHT 2015 ARM Limited. All rights reserved.
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



/* This implements a wrapper around the DRM slave encoder framework so that
 * slave encoder drivers can be used as video-tx drivers
 */

#define DEBUG

#include <linux/module.h>
#include <linux/of_i2c.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_encoder_slave.h>

#include <video/video_tx.h>

#define DEFAULT_ENCODER_TYPE   DRM_MODE_ENCODER_TMDS
#define DEFAULT_CONNECTOR_TYPE DRM_MODE_CONNECTOR_HDMIA

struct senc_tx {
	struct device *dev;
	struct i2c_client *client;
	/* Skeleton DRM objects */
	struct drm_device drm_dev;
	struct drm_encoder_slave drm_slave;
	struct drm_connector drm_conn;
	/* Video-tx objects */
	struct video_tx_device *vtx_dev;
	struct video_tx_info tx_info;
	/* Mutex to make sure we are thread-safe with the polling thread */
	struct mutex mode_mutex;
	enum drm_connector_status connector_status;
	int n_modes;
};

static void drm_display_mode_to_modeinfo(struct drm_display_mode *disp_mode,
	struct drm_mode_modeinfo *modeinfo)
{
	modeinfo->clock = disp_mode->clock;
	modeinfo->hdisplay = disp_mode->hdisplay;
	modeinfo->hsync_start = disp_mode->hsync_start;
	modeinfo->hsync_end = disp_mode->hsync_end;
	modeinfo->htotal = disp_mode->htotal;
	modeinfo->hskew = disp_mode->hskew;
	modeinfo->vdisplay = disp_mode->vdisplay;
	modeinfo->vsync_start = disp_mode->vsync_start;
	modeinfo->vsync_end = disp_mode->vsync_end;
	modeinfo->vtotal = disp_mode->vtotal;
	modeinfo->vscan = disp_mode->vscan;
	modeinfo->vrefresh = disp_mode->vrefresh;

	modeinfo->flags = disp_mode->flags;
	modeinfo->type = disp_mode->type;

	strncpy(modeinfo->name, disp_mode->name, DRM_DISPLAY_MODE_LEN);
}

static void drm_modeinfo_to_display_mode(struct drm_mode_modeinfo *modeinfo,
	struct drm_display_mode *disp_mode)
{
	disp_mode->clock = modeinfo->clock;
	disp_mode->hdisplay = modeinfo->hdisplay;
	disp_mode->hsync_start = modeinfo->hsync_start;
	disp_mode->hsync_end = modeinfo->hsync_end;
	disp_mode->htotal = modeinfo->htotal;
	disp_mode->hskew = modeinfo->hskew;
	disp_mode->vdisplay = modeinfo->vdisplay;
	disp_mode->vsync_start = modeinfo->vsync_start;
	disp_mode->vsync_end = modeinfo->vsync_end;
	disp_mode->vtotal = modeinfo->vtotal;
	disp_mode->vscan = modeinfo->vscan;
	disp_mode->vrefresh = modeinfo->vrefresh;

	disp_mode->flags = modeinfo->flags;
	disp_mode->type = modeinfo->type;

	strncpy(disp_mode->name, modeinfo->name, DRM_DISPLAY_MODE_LEN);
}

static int senc_tx_get_modes(struct video_tx_device *vtx_dev,
	struct drm_mode_modeinfo *mode_info, int n)
{
	struct senc_tx *tx = video_tx_get_drvdata(vtx_dev);
	struct drm_display_mode *p;
	int min, i = 0;

	mutex_lock(&tx->mode_mutex);
	min = n < tx->n_modes ? n : tx->n_modes;

	if (mode_info == NULL) {
		dev_dbg(tx->dev, "%s: %d modes available\n", __func__,
			tx->n_modes);
		mutex_unlock(&tx->mode_mutex);
		return tx->n_modes;
	}

	dev_dbg(tx->dev, "%s: copying %d modes\n", __func__, min);
	list_for_each_entry(p, &tx->drm_conn.modes, head) {
		struct drm_mode_modeinfo *m = &mode_info[i++];
		drm_display_mode_to_modeinfo(p, m);
	}
	mutex_unlock(&tx->mode_mutex);
	return min;
}

static int senc_tx_set_mode(struct video_tx_device *vtx_dev,
	struct drm_mode_modeinfo *modeinfo)
{
	struct senc_tx *tx = video_tx_get_drvdata(vtx_dev);
	struct drm_encoder_slave_funcs *slave_funcs =
		tx->drm_slave.slave_funcs;
	struct drm_display_mode disp_mode;

	memset(&disp_mode, 0, sizeof(disp_mode));

	drm_modeinfo_to_display_mode(modeinfo, &disp_mode);

	dev_dbg(tx->dev, "%s: Setting %s\n", __func__, modeinfo->name);
	dev_dbg(tx->dev, "%s: %d %d %d %d %d\n", __func__,
		modeinfo->hdisplay, modeinfo->hsync_start, modeinfo->hsync_end,
		modeinfo->htotal, modeinfo->hskew);
	dev_dbg(tx->dev, "%s: %d %d %d %d %d\n", __func__,
		modeinfo->vdisplay, modeinfo->vsync_start, modeinfo->vsync_end,
		modeinfo->vtotal, modeinfo->vscan);
	dev_dbg(tx->dev, "%s: vrefresh: %d flags: 0x%x type: 0x%x\n",
		__func__, modeinfo->vrefresh, modeinfo->flags, modeinfo->type);

	slave_funcs->mode_set(&tx->drm_slave.base, &disp_mode, &disp_mode);

	return 0;
}

static int senc_tx_dpms(struct video_tx_device *vtx_dev, int mode)
{
	struct senc_tx *tx = video_tx_get_drvdata(vtx_dev);
	struct drm_encoder_slave_funcs *slave_funcs =
		tx->drm_slave.slave_funcs;

	slave_funcs->dpms(&tx->drm_slave.base, mode);

	return 0;
}

static int senc_tx_get_tx_info(struct video_tx_device *dev,
	struct video_tx_info *tx_info)
{
	struct senc_tx *tx = video_tx_get_drvdata(dev);

	if (tx_info == NULL)
		return -ENOMEM;

	memcpy(tx_info, &tx->tx_info,
	       sizeof(struct video_tx_info));

	return 0;
}

/* Should be called with the tx->mode_mutex held
 * This broadly copies drm_helper_probe_single_connector_modes
 */
static void senc_tx_handle_hotplug(struct senc_tx *tx,
	enum drm_connector_status status)
{
	struct drm_encoder_slave_funcs *slave_funcs =
		tx->drm_slave.slave_funcs;
	struct drm_display_mode *mode;

	if (status == connector_status_connected) {
		int n_probed, n = 0;

		n_probed = slave_funcs->get_modes(&tx->drm_slave.base,
			&tx->drm_conn);
		/* If we couldn't get any, add some default DMT ones */
		if (!n_probed)
			drm_add_modes_noedid(&tx->drm_conn, 1024, 768);

		/* Copy the probed modes into the modes list */
		drm_mode_connector_list_update(&tx->drm_conn);

		/* Validate the probed modes */
		list_for_each_entry(mode, &tx->drm_conn.modes, head) {
			if ((mode->flags & DRM_MODE_FLAG_INTERLACE) &&
			    !(tx->drm_conn.interlace_allowed))
				mode->status = MODE_NO_INTERLACE;

			if ((mode->flags & DRM_MODE_FLAG_DBLSCAN) &&
			    !(tx->drm_conn.doublescan_allowed))
				mode->status = MODE_NO_DBLESCAN;

			if ((mode->status == MODE_OK) && slave_funcs &&
			    slave_funcs->mode_valid)
				mode->status = slave_funcs->mode_valid(&tx->drm_slave.base,
					mode);
		}
		dev_dbg(tx->dev, "%s: Probed %d modes\n", __func__, n_probed);

		drm_mode_prune_invalid(&tx->drm_dev, &tx->drm_conn.modes,
				       false);
		if (!list_empty(&tx->drm_conn.modes)) {
			drm_mode_sort(&tx->drm_conn.modes);
			list_for_each_entry(mode, &tx->drm_conn.modes, head) {
				mode->vrefresh = drm_mode_vrefresh(mode);
				drm_mode_debug_printmodeline(mode);
				n++;
			}
		}
		tx->n_modes = n;
		dev_dbg(tx->dev, "%s: Have %d valid modes\n", __func__, n);
	} else {
		struct drm_display_mode *t;
		/* Wipe out the current mode lists */
		list_for_each_entry_safe(mode, t, &tx->drm_conn.probed_modes, head)
			drm_mode_remove(&tx->drm_conn, mode);
		list_for_each_entry_safe(mode, t, &tx->drm_conn.modes, head)
			drm_mode_remove(&tx->drm_conn, mode);
		tx->n_modes = 0;

		drm_mode_connector_update_edid_property(&tx->drm_conn, NULL);
	}
}

static enum drm_connector_status senc_tx_detect(struct video_tx_device *vtx_dev)
{
	struct senc_tx *tx = video_tx_get_drvdata(vtx_dev);
	struct drm_encoder_slave_funcs *slave_funcs =
		tx->drm_slave.slave_funcs;
	enum drm_connector_status status = connector_status_unknown;

	mutex_lock(&tx->mode_mutex);

	if (slave_funcs->detect)
		status = slave_funcs->detect(&tx->drm_slave.base,
				&tx->drm_conn);

	if (status != tx->connector_status) {
		dev_dbg(tx->dev, "%s: Status change: %s->%s\n", __func__,
			drm_get_connector_status_name(tx->connector_status),
			drm_get_connector_status_name(status));
		tx->connector_status = status;

		senc_tx_handle_hotplug(tx, status);
	}

	mutex_unlock(&tx->mode_mutex);

	return status;
}

static int senc_tx_get_display_info(struct video_tx_device *vtx_dev,
	struct video_tx_display_info *disp_info)
{
	struct senc_tx *tx = video_tx_get_drvdata(vtx_dev);
	struct edid *edid =  NULL;

	mutex_lock(&tx->mode_mutex);

	if (!tx->drm_conn.edid_blob_ptr) {
		dev_warn(tx->dev, "%s: No edid property available. Using defaults\n",
			 __func__);
		/* Gamma = 2.2 */
		disp_info->gamma = 120;
		/* sRGB color space coordinates */
		disp_info->red_x = 655;
		disp_info->red_y = 338;
		disp_info->blue_x = 154;
		disp_info->blue_y = 61;
		disp_info->green_x = 307;
		disp_info->green_y = 614;
		disp_info->white_x = 320;
		disp_info->white_y = 337;
		mutex_unlock(&tx->mode_mutex);
		return 0;
	}

	edid = (struct edid *)tx->drm_conn.edid_blob_ptr->data;

	disp_info->gamma = edid->gamma;

	disp_info->red_x =
		(edid->red_x << 2) | ((edid->red_green_lo >> 6) & 0x3);
	disp_info->red_y =
		(edid->red_y << 2) | ((edid->red_green_lo >> 4) & 0x3);
	disp_info->green_x =
		(edid->green_x << 2) | ((edid->red_green_lo >> 2) & 0x3);
	disp_info->green_y =
		(edid->green_y << 2) | ((edid->red_green_lo >> 0) & 0x3);

	disp_info->blue_x =
		(edid->blue_x << 2) | ((edid->black_white_lo >> 6) & 0x3);
	disp_info->blue_y =
		(edid->blue_y << 2) | ((edid->black_white_lo >> 4) & 0x3);
	disp_info->white_x =
		(edid->white_x << 2) | ((edid->black_white_lo >> 2) & 0x3);
	disp_info->white_y =
		(edid->white_y << 2) | ((edid->black_white_lo >> 0) & 0x3);

	disp_info->width_mm = tx->drm_conn.display_info.width_mm;
	disp_info->height_mm = tx->drm_conn.display_info.height_mm;

	mutex_unlock(&tx->mode_mutex);

	return 0;
}

static struct video_tx_driver senc_vtx_driver = {
	.get_modes = senc_tx_get_modes,
	.set_mode = senc_tx_set_mode,
	.dpms = senc_tx_dpms,
	.get_tx_info = senc_tx_get_tx_info,
	.detect = senc_tx_detect,
	.get_display_info = senc_tx_get_display_info,
};

static void senc_tx_drm_dev_init(struct senc_tx *tx)
{
	struct drm_device *dev = &tx->drm_dev;

	dev->dev = tx->dev;

	drm_mode_config_init(&tx->drm_dev);
}

static int senc_tx_drm_slave_init(struct senc_tx *tx,
		struct i2c_board_info *i2c_info)
{
	int ret;

	ret = drm_encoder_init(&tx->drm_dev, &tx->drm_slave.base, NULL,
		DEFAULT_ENCODER_TYPE);
	if (ret) {
		dev_err(tx->dev, "%s: encoder_init failed\n", __func__);
		return ret;
	}

	/* LEOSW-408: drm_i2c_encoder_attach is what we want actually:
	 * http://lists.freedesktop.org/archives/dri-devel/2015-January/075409.html
	 */
	ret = drm_i2c_encoder_init(&tx->drm_dev, &tx->drm_slave, NULL,
			i2c_info);
	if (ret) {
		dev_err(tx->dev, "%s: i2c_encoder_init failed\n", __func__);
		goto fail_encoder_cleanup;
	}

	return 0;

fail_encoder_cleanup:
	drm_encoder_cleanup(&tx->drm_slave.base);
	return ret;
}

static void senc_tx_drm_slave_exit(struct senc_tx *tx)
{
	if (tx->drm_slave.slave_funcs && tx->drm_slave.slave_funcs->destroy)
		tx->drm_slave.slave_funcs->destroy(&tx->drm_slave.base);
	drm_encoder_cleanup(&tx->drm_slave.base);
}

static int senc_tx_drm_conn_init(struct senc_tx *tx)
{
	int ret;

	tx->drm_conn.interlace_allowed = false;
	tx->drm_conn.doublescan_allowed = false;
	ret = drm_connector_init(&tx->drm_dev, &tx->drm_conn, NULL,
			tx->tx_info.connector_type);
	if (ret) {
		dev_err(tx->dev, "%s: connector_init failed\n", __func__);
		return ret;
	}

	return 0;
}

static int senc_tx_drm_init(struct senc_tx *tx,
		struct i2c_board_info *i2c_info)
{
	int ret;

	senc_tx_drm_dev_init(tx);

	ret = senc_tx_drm_slave_init(tx, i2c_info);
	if (ret)
		return ret;

	ret = senc_tx_drm_conn_init(tx);
	if (ret) {
		senc_tx_drm_slave_exit(tx);
		return ret;
	}

	return 0;
}

static void senc_tx_drm_exit(struct senc_tx *tx)
{
	drm_connector_cleanup(&tx->drm_conn);
	senc_tx_drm_slave_exit(tx);
}

#define MAX_TYPE_STRLEN 5
static int senc_tx_conn_type(const char *str, u32 *type)
{
	int i;
	struct {
		char str[MAX_TYPE_STRLEN];
		u32 type;
	} connector_types[] = {
		{ "HDMI", DRM_MODE_CONNECTOR_HDMIA },
		{ "DVI", DRM_MODE_CONNECTOR_DVII },
	};

	for (i = 0; i < ARRAY_SIZE(connector_types); i++) {
		if (!strncmp(connector_types[i].str, str, MAX_TYPE_STRLEN)) {
			*type = connector_types[i].type;
			return 0;
		}
	}

	return -EINVAL;
}

static int senc_tx_parse_dt(struct senc_tx *tx, struct device_node *np)
{
	int ret;
	const char *str;
	struct device_node *slave_node;
	phandle slave_phandle;

	if (of_property_read_u32(np, "i2c-slave", &slave_phandle)) {
		dev_err(tx->dev, "no i2c-slave handle provided!\n");
		return -ENODEV;
	} else {
		slave_node = of_find_node_by_phandle(slave_phandle);
		BUG_ON(!slave_node);
	}
	tx->client = of_find_i2c_device_by_node(slave_node);
	of_node_put(slave_node);
	if (!tx->client) {
		dev_err(tx->dev, "Couldn't find i2c client\n");
		return -ENODEV;
	}

	/* Init video tx info */
	ret = of_property_read_string(np, "type", &str);
	if (ret) {
		dev_warn(tx->dev, "%s: type not found. Using default\n",
			 __func__);
		tx->tx_info.connector_type = DEFAULT_CONNECTOR_TYPE;
	} else {
		ret = senc_tx_conn_type(str, &tx->tx_info.connector_type);
		if (ret) {
			dev_err(tx->dev, "%s: unrecognized output type '%s'\n",
				__func__, str);
			return ret;
		}
	}

	ret = of_property_read_u32(np, "type-idx", &tx->tx_info.idx);
	if (ret) {
		tx->tx_info.idx = np->phandle;
		dev_warn(tx->dev, "%s: type-idx not found. Using phandle\n",
			 __func__);
	}

	snprintf(tx->tx_info.name, DRM_CONNECTOR_NAME_LEN, "%s",
		 dev_name(tx->dev));

	/* LEOSW-188 This can actually change at runtime! */
	tx->tx_info.red_bits = 8;
	tx->tx_info.green_bits = 8;
	tx->tx_info.blue_bits = 8;

	return 0;
}

static int senc_tx_probe(struct platform_device *pdev)
{
	int ret;
	struct senc_tx *tx;
	struct device_node *np = pdev->dev.of_node;
	struct i2c_board_info i2c_info;

	if (!np) {
		dev_err(&pdev->dev, "only device tree set up supported\n");
		return -ENODEV;
	}

	tx = devm_kzalloc(&pdev->dev, sizeof(*tx), GFP_KERNEL);
	if (!tx)
		return -ENOMEM;

	tx->dev = &pdev->dev;
	tx->connector_status = connector_status_unknown;
	mutex_init(&tx->mode_mutex);

	ret = senc_tx_parse_dt(tx, np);
	if (ret)
		return ret;

	/* get the driver for the i2c slave node */
	memset(&i2c_info, 0, sizeof(i2c_info));
	i2c_info.of_node = tx->client->dev.of_node;
	ret = of_modalias_node(i2c_info.of_node, i2c_info.type,
		sizeof(i2c_info.type));
	if (ret < 0) {
		dev_err(tx->dev, "failed to get a module alias for node %s\n",
			i2c_info.of_node->full_name);
	}

	ret = senc_tx_drm_init(tx, &i2c_info);
	if (ret)
		return ret;

	tx->vtx_dev = video_tx_register_device(tx->dev, &senc_vtx_driver);
	if (!tx->vtx_dev) {
		senc_tx_drm_exit(tx);
		dev_err(tx->dev, "%s: Failed to register with video-tx framework\n",
			__func__);
		return -EINVAL;
	}

	platform_set_drvdata(pdev, tx);
	video_tx_set_drvdata(tx->vtx_dev, tx);

	video_tx_request_polling(tx->vtx_dev);

	dev_dbg(tx->dev, "Probed device '%s'", tx->tx_info.name);

	return 0;
}

static int senc_tx_remove(struct platform_device *pdev)
{
	struct senc_tx *tx = platform_get_drvdata(pdev);

	video_tx_cancel_polling(tx->vtx_dev);
	senc_tx_dpms(tx->vtx_dev, DRM_MODE_DPMS_OFF);

	video_tx_unregister_device(tx->vtx_dev);
	senc_tx_drm_exit(tx);
	return 0;
}

static const struct of_device_id slave_enc_tx_ids[] = {
	{ .compatible = "generic,slave_enc_video_tx" },
	{ /*empty*/ }
};

static struct platform_driver slave_enc_tx_driver = {
	.probe = senc_tx_probe,
	.remove = senc_tx_remove,
	.driver = {
		.name = "senc_tx",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(slave_enc_tx_ids),
	},
};

module_platform_driver(slave_enc_tx_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DRM slave encoder Video-Tx driver");
