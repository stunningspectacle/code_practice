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



#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

#include <video/video_tx.h>

#define TEST_VD_TX_DEV	"fake_vd_tx"

struct video_tx_client_data {
	struct mutex tx_client_lock;
	struct device *dev;
	u16 current_mode_index;
	struct video_tx_display_info dummy_display_info;
	struct video_tx_info dummy_tx_info;
	int dpms_status;
	enum drm_connector_status conn_status;
	bool polling_enabled;
};

/* internal func */
static const char *conn_status_str(enum drm_connector_status c)
{
	const char *p;

	switch (c) {
	case connector_status_connected:
		p = "connected";
		break;
	case connector_status_disconnected:
		p = "disconnected";
		break;
	case connector_status_unknown:
		p = "unknown";
		break;
	default:
		p = "connect error";
	}
	return p;
}

static struct video_tx_client_data *get_from_dev(struct device *dev)
{
	struct video_tx_client_data *vcd;
	struct video_tx_device *vtd;

	vtd = dev_get_drvdata(dev);
	vcd = video_tx_get_drvdata(vtd);
	return vcd;
}

/* sysfs file for connection */
static ssize_t show_connect(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);

	return scnprintf(buf, 20, "%s\n",
		conn_status_str(video_tx_client->conn_status));
}

static ssize_t store_connect(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	enum drm_connector_status c;
	const char *p;
	struct video_tx_device *tx_device = dev_get_drvdata(dev);
	struct video_tx_client_data *video_tx_client;
	bool report_hotplug_event = false;

	video_tx_client = video_tx_get_drvdata(tx_device);
	/*NOTE: only match 7 characters from the begining of the string */
	mutex_lock(&video_tx_client->tx_client_lock);
	for (c = connector_status_connected;
			c <= connector_status_unknown && count >= 7;
			c++) {
		p = conn_status_str(c);
		if (strncasecmp(buf, p, 7) == 0)
			break;
	}

	if (video_tx_client->conn_status != c) {
		video_tx_client->conn_status = c;
		if (video_tx_client->polling_enabled == false)
			report_hotplug_event = true;
	}
	mutex_unlock(&video_tx_client->tx_client_lock);

	if (report_hotplug_event)
		video_tx_report_hotplug_event(tx_device);
	return count;
}

/* sysfs file for gamma */
static ssize_t show_gamma(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);

	return scnprintf(buf, 20, "%hhu\n",
			video_tx_client->dummy_display_info.gamma);
}

static ssize_t store_gamma(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);

	mutex_lock(&video_tx_client->tx_client_lock);
	if (sscanf(buf, "%hhu", &video_tx_client->dummy_display_info.gamma) != 1)
		dev_err(dev, "input gamma error\n");
	mutex_unlock(&video_tx_client->tx_client_lock);

	return count;
}

/* sysfs file for polling */
#define STR_POLLING_EN	"enabled"
#define STR_POLLING_DIS	"disabled"

static ssize_t show_polling(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);

	return scnprintf(buf, 32, "%s\n",
				(video_tx_client->polling_enabled == true) ?
				STR_POLLING_EN : STR_POLLING_DIS);
}

static ssize_t store_polling(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct video_tx_device *tx_device = dev_get_drvdata(dev);
	struct video_tx_client_data *video_tx_client;
	bool polling_en;

	video_tx_client = video_tx_get_drvdata(tx_device);
	if (count == 0 || video_tx_client == NULL)
		return -EINVAL;

	if (strncasecmp(buf, STR_POLLING_EN, strlen(STR_POLLING_EN)) == 0)
		polling_en = true;
	else if (strncasecmp(buf, STR_POLLING_DIS,
			strlen(STR_POLLING_DIS)) == 0)
		polling_en = false;
	else
		return -EINVAL;

	mutex_lock(&video_tx_client->tx_client_lock);

	if (polling_en != video_tx_client->polling_enabled) {
		video_tx_client->polling_enabled = polling_en;
		if (polling_en == true)
			video_tx_request_polling(tx_device);
		else
			video_tx_cancel_polling(tx_device);
	}

	mutex_unlock(&video_tx_client->tx_client_lock);

	return count;
}

/* sysfs files for color space coordinates */
enum color_id {
	color_id_red = 0,
	color_id_green,
	color_id_blue,
	color_id_white
};

static ssize_t show_color_xy(u16 color_x, u16 color_y, char *buf)
{
	/* show coordinates as 10bits format */
	u32 x = color_x;
	u32 y = color_y;

	return scnprintf(buf, 32, "%u,%u\n", x, y);
}

static ssize_t store_color_xy(const char *buf, size_t count,
	u16 *color_x, u16 *color_y)
{
	u32 x, y;

	if (sscanf(buf, "%u,%u", &x, &y) != 2)
		return 0;

	if (x > 1023 || y > 1023)
		return 0;

	*color_x = x;
	*color_y = y;
	return count;
}

static ssize_t show_color(struct device *dev, char *buf,
	enum color_id color)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);
	u16 color_x, color_y;

	mutex_lock(&video_tx_client->tx_client_lock);
	switch (color) {
	case color_id_red:
		color_x = video_tx_client->dummy_display_info.red_x;
		color_y = video_tx_client->dummy_display_info.red_y;
		break;
	case color_id_green:
		color_x = video_tx_client->dummy_display_info.green_x;
		color_y = video_tx_client->dummy_display_info.green_y;
		break;
	case color_id_blue:
		color_x = video_tx_client->dummy_display_info.blue_x;
		color_y = video_tx_client->dummy_display_info.blue_y;
		break;
	case color_id_white:
		color_x = video_tx_client->dummy_display_info.white_x;
		color_y = video_tx_client->dummy_display_info.white_y;
		break;
	default:
		BUG();
	}
	mutex_unlock(&video_tx_client->tx_client_lock);

	return show_color_xy(color_x, color_y, buf);
}

static ssize_t store_color(struct device *dev, const char *buf,
	size_t count, enum color_id color)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);
	size_t ret;
	u16 *color_x, *color_y;

	switch (color) {
	case color_id_red:
		color_x = &video_tx_client->dummy_display_info.red_x;
		color_y = &video_tx_client->dummy_display_info.red_y;
		break;
	case color_id_green:
		color_x = &video_tx_client->dummy_display_info.green_x;
		color_y = &video_tx_client->dummy_display_info.green_y;
		break;
	case color_id_blue:
		color_x = &video_tx_client->dummy_display_info.blue_x;
		color_y = &video_tx_client->dummy_display_info.blue_y;
		break;
	case color_id_white:
		color_x = &video_tx_client->dummy_display_info.white_x;
		color_y = &video_tx_client->dummy_display_info.white_y;
		break;
	default:
		BUG();
	}
	mutex_lock(&video_tx_client->tx_client_lock);
	ret = store_color_xy(buf, count, color_x, color_y);
	mutex_unlock(&video_tx_client->tx_client_lock);

	return (ret == 0) ? -EINVAL : ret;
}

static ssize_t show_red(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	return show_color(dev, buf, color_id_red);
}

static ssize_t store_red(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return store_color(dev, buf, count, color_id_red);
}

static ssize_t show_green(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	return show_color(dev, buf, color_id_green);
}

static ssize_t store_green(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return store_color(dev, buf, count, color_id_green);
}

static ssize_t show_blue(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	return show_color(dev, buf, color_id_blue);
}

static ssize_t store_blue(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return store_color(dev, buf, count, color_id_blue);
}

static ssize_t show_white(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	return show_color(dev, buf, color_id_white);
}

static ssize_t store_white(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return store_color(dev, buf, count, color_id_white);
}

static ssize_t show_dpms(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct video_tx_client_data *video_tx_client = get_from_dev(dev);
	int state;
	mutex_lock(&video_tx_client->tx_client_lock);
	state = video_tx_client->dpms_status;
	mutex_unlock(&video_tx_client->tx_client_lock);

	return scnprintf(buf, PAGE_SIZE, "%u\n", state);
}

#define attr_mode (S_IWUGO | S_IRUGO)
DEVICE_ATTR(disp_connect, attr_mode, show_connect, store_connect);
DEVICE_ATTR(gamma, attr_mode, show_gamma, store_gamma);
DEVICE_ATTR(polling, attr_mode, show_polling, store_polling);
DEVICE_ATTR(red_coords, attr_mode, show_red, store_red);
DEVICE_ATTR(green_coords, attr_mode, show_green, store_green);
DEVICE_ATTR(blue_coords, attr_mode, show_blue, store_blue);
DEVICE_ATTR(white_coords, attr_mode, show_white, store_white);
DEVICE_ATTR(dpms, S_IRUGO, show_dpms, NULL);

/* attr group */
static struct attribute *attr_list[] = {
	&dev_attr_disp_connect.attr,
	&dev_attr_gamma.attr,
	&dev_attr_polling.attr,
	&dev_attr_red_coords.attr,
	&dev_attr_green_coords.attr,
	&dev_attr_blue_coords.attr,
	&dev_attr_white_coords.attr,
	&dev_attr_dpms.attr,
	NULL
};

static struct attribute_group fake_v_tx_attr_group = {
	.attrs = attr_list,
};

/* Data zone for a dummy display */
const struct drm_mode_modeinfo modelist[] = {
	{ /* 640x480 */
		.clock = 25175,
		.hdisplay = 640,
		.hsync_start = 656,
		.hsync_end = 752,
		.htotal = 800,
		.hskew = 0,
		.vdisplay = 480,
		.vsync_start = 490,
		.vsync_end = 492,
		.vtotal = 525,
		.vscan = 0,

		.vrefresh = 60,

		.type = DRM_MODE_TYPE_PREFERRED,
		.name = {"640x480-60"},
	},
	{ /* 1024x768 */
		.clock = 65000,
		.hdisplay = 1024,
		.hsync_start = 1048,
		.hsync_end = 1184,
		.htotal = 1344,
		.hskew = 0,
		.vdisplay = 768,
		.vsync_start = 771,
		.vsync_end = 777,
		.vtotal = 806,
		.vscan = 0,

		.vrefresh = 60,

		.name = {"1024x768-60"},
	},
	{ /* 640x480 @30Hz */
		.clock = 12500,
		.hdisplay = 640,
		.hsync_start = 656,
		.hsync_end = 752,
		.htotal = 800,
		.hskew = 0,
		.vdisplay = 480,
		.vsync_start = 490,
		.vsync_end = 492,
		.vtotal = 525,
		.vscan = 0,

		.vrefresh = 30,

		.name = {"640x480-30"},
	},
	{ /* 1920x1080 @24Hz */
		.clock = 74250,
		.hdisplay = 1920,
		.hsync_start = 2558,
		.hsync_end = 2602,
		.htotal = 2750,
		.hskew = 0,
		.vdisplay = 1080,
		.vsync_start = 1084,
		.vsync_end = 1089,
		.vtotal = 1125,
		.vscan = 0,

		.vrefresh = 24,

		.name = {"1920x1080-24"},
	},
};


/* API for transmitter driver */
static int test_get_modes(struct video_tx_device *dev,
			  struct drm_mode_modeinfo *mode_info, int n)
{
	struct video_tx_client_data *video_tx_client =
				video_tx_get_drvdata(dev);
	int min = n < ARRAY_SIZE(modelist) ? n : ARRAY_SIZE(modelist);

	if (video_tx_client->conn_status != connector_status_connected)
		return -EINVAL;
	if (mode_info == NULL)
		return ARRAY_SIZE(modelist);

	memcpy(mode_info, modelist, sizeof(modelist[0]) * min);
	dev_dbg(video_tx_client->dev, "driver->test_get_modes_list\n");
	return ARRAY_SIZE(modelist);
}

static int test_set_mode(struct video_tx_device *dev,
				struct drm_mode_modeinfo *mode_info)
{
	int i = -1;
	struct video_tx_client_data *video_tx_client =
				video_tx_get_drvdata(dev);

	if (video_tx_client->conn_status != connector_status_connected)
		return -EINVAL;
	if (mode_info == NULL)
		return -EINVAL;

	mutex_lock(&video_tx_client->tx_client_lock);
	for (i = 0; i < ARRAY_SIZE(modelist); i++) {
		if (memcmp(mode_info, modelist+i, sizeof(*mode_info)) == 0) {
			video_tx_client->current_mode_index = i;
			break;
		}
	}
	mutex_unlock(&video_tx_client->tx_client_lock);

	dev_dbg(video_tx_client->dev, "driver->test_set_mode\n");

	return (i < ARRAY_SIZE(modelist)) ? 0 : -EINVAL;
}

static int test_dpms(struct video_tx_device *dev, int status)
{
	char *str_dpms;
	struct video_tx_client_data *video_tx_client =
				video_tx_get_drvdata(dev);

	switch (status) {
	case DRM_MODE_DPMS_ON:
		str_dpms = "DRM_MODE_DPMS_ON";
		break;
	case DRM_MODE_DPMS_STANDBY:
		str_dpms = "DRM_MODE_DPMS_STANDBY";
		break;
	case DRM_MODE_DPMS_SUSPEND:
		str_dpms = "DRM_MODE_DPMS_SUSPEND";
		break;
	case DRM_MODE_DPMS_OFF:
		str_dpms = "DRM_MODE_DPMS_OFF";
		break;
	default:
		dev_err(video_tx_client->dev, "DPMS: unknown status!\n");
		return -EINVAL;
	}

	mutex_lock(&video_tx_client->tx_client_lock);
	video_tx_client->dpms_status = status;
	dev_dbg(video_tx_client->dev, "driver->dpms( %s )\n",
			str_dpms);
	mutex_unlock(&video_tx_client->tx_client_lock);

	return 0;
}

static int test_get_tx_info(struct video_tx_device *dev,
				struct video_tx_info *tx_info)
{
	struct video_tx_client_data *video_tx_client =
				video_tx_get_drvdata(dev);

	if (tx_info == NULL)
		return -ENOMEM;

	memcpy(tx_info, &video_tx_client->dummy_tx_info,
			sizeof(struct video_tx_info));
	dev_dbg(video_tx_client->dev, "driver->get_tx_info\n");
	return 0;
}

static int test_get_display_info(struct video_tx_device *dev,
		struct video_tx_display_info *disp_info)
{
	struct video_tx_client_data *video_tx_client =
				video_tx_get_drvdata(dev);

	if (disp_info == NULL)
		return -ENOMEM;
	if (video_tx_client->conn_status != connector_status_connected)
		return -EINVAL;

	memcpy(disp_info, &video_tx_client->dummy_display_info,
			sizeof(struct video_tx_display_info));
	dev_dbg(video_tx_client->dev, "driver->get_display_info\n");
	return 0;
}

static enum drm_connector_status test_detect(
				struct video_tx_device *dev)
{
	struct video_tx_client_data *video_tx_client =
			video_tx_get_drvdata(dev);

	dev_dbg(video_tx_client->dev, "driver->detect\n");
	return video_tx_client->conn_status;
}

static struct video_tx_driver test_driver_tx = {
	.get_modes = test_get_modes,
	.set_mode = test_set_mode,
	.dpms = test_dpms,
	.get_tx_info = test_get_tx_info,
	.detect = test_detect,
	.get_display_info = test_get_display_info
};

/* methods for video_tx_client */
static int client_init(struct device *dev)
{
	int ret = 0;
	struct video_tx_client_data	*video_tx_client;
	struct video_tx_device *v_tx_dev;

	video_tx_client = devm_kzalloc(dev, sizeof(*video_tx_client),
						GFP_KERNEL);
	if (video_tx_client == NULL)
		return -ENOMEM;

	mutex_init(&video_tx_client->tx_client_lock);

	snprintf(video_tx_client->dummy_tx_info.name,
			DRM_CONNECTOR_NAME_LEN,
			"Generic Video");
	video_tx_client->dummy_tx_info.connector_type =
				DRM_MODE_CONNECTOR_VGA;
	video_tx_client->dummy_tx_info.idx = 0;
	video_tx_client->dummy_tx_info.red_bits = 8;
	video_tx_client->dummy_tx_info.green_bits = 8;
	video_tx_client->dummy_tx_info.blue_bits = 8;

	video_tx_client->dummy_display_info.gamma = 0; /* real gamma 1.0 */

	/* Default color space coordinates (10bits)
	 * The data comes from DELL 1930MWc
	 */
	video_tx_client->dummy_display_info.red_x = 655;
	video_tx_client->dummy_display_info.red_y = 338;
	video_tx_client->dummy_display_info.blue_x = 154;
	video_tx_client->dummy_display_info.blue_y = 61;
	video_tx_client->dummy_display_info.green_x = 307;
	video_tx_client->dummy_display_info.green_y = 614;
	video_tx_client->dummy_display_info.white_x = 321;
	video_tx_client->dummy_display_info.white_y = 337;

	/*
	 * Because the resolution used by FPGA is 640x480, for generating
	 * a proper dpi for android home screen, we set the example display
	 * size to 3.7 inch.
	 */
	video_tx_client->dummy_display_info.width_mm = 76;
	video_tx_client->dummy_display_info.height_mm = 57;

	video_tx_client->conn_status = connector_status_connected;
	video_tx_client->dev = dev;

	ret = sysfs_create_group(&dev->kobj,
					&fake_v_tx_attr_group);
	if (ret) {
		dev_err(dev, "sysfs for video tx client error\n");
		return ret;
	}

	v_tx_dev = video_tx_register_device(dev, &test_driver_tx);
	if (v_tx_dev == NULL) {
		dev_err(dev, "register video tx device error.\n");
		return -EINVAL;
	}

	ret = dev_set_drvdata(dev, v_tx_dev);
	if (ret) {
		video_tx_unregister_device(v_tx_dev);
		return ret;
	}

	video_tx_client->polling_enabled = true;
	video_tx_set_drvdata(v_tx_dev, video_tx_client);
	video_tx_request_polling(v_tx_dev);

	return ret;
}

static void client_destroy(struct device *dev)
{
	struct video_tx_device *v_tx_dev;
	struct video_tx_client_data *video_tx_client;

	sysfs_remove_group(&dev->kobj, &fake_v_tx_attr_group);

	v_tx_dev = dev_get_drvdata(dev);
	if (v_tx_dev != NULL) {
		video_tx_client = video_tx_get_drvdata(v_tx_dev);
		if (video_tx_client->polling_enabled == true)
			video_tx_cancel_polling(v_tx_dev);
		video_tx_unregister_device(v_tx_dev);
	}
}

/* module entry and exit functions */
static int video_tx_probe(struct platform_device *pdev)
{
	int ret = client_init(&pdev->dev);

	if (ret != 0)
		return ret;

	dev_dbg(&pdev->dev, "fake video tx client driver is loaded.\n");
	return 0;
}

static int video_tx_remove(struct platform_device *pdev)
{
	client_destroy(&pdev->dev);
	dev_dbg(&pdev->dev, "fake video tx client driver is removed.\n");
	return 0;
}

static const struct of_device_id video_transmitter_ids[] = {
	{ .compatible = "generic,video_transmitter" },
	{ /*empty*/ }
};

static struct platform_driver f_video_tx_client = {
	.probe = video_tx_probe,
	.remove = video_tx_remove,
	.driver = {
		.name = TEST_VD_TX_DEV,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(video_transmitter_ids),
	},
};

module_platform_driver(f_video_tx_client);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fake_Video_TX_driver");
