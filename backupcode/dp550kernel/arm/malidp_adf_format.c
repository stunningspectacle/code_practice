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



#include "malidp_adf.h"
#include "malidp_adf_format.h"
#include <video/adf_format.h>
#include <uapi/video/malidp_adf.h>

#define AFBC_HEADER_SIZE 16
#define AFBC_SUPERBLOCK_SIZE 16

/** Validate a buffer using a tiled pixel format
 *
 * @dev: ADF device performing the validation
 * @buf: buffer to validate
 * @num_planes: expected number of planes
 * @tile_w: the width in pixels of each tile
 * @tile_h: the height in pixels of each tile
 * @cpt: expected bytes per tile for each plane (length @num_planes)
 *
 * Returns 0 if @buf has the expected number of planes and each plane
 * has sufficient size, or -EINVAL otherwise.
 */
static int malidp_adf_format_validate_tiled(struct adf_device *dev,
	struct adf_buffer *buf, u8 num_planes, u8 tile_w, u8 tile_h, u8 cpt[])
{
	u8 i;

	if (num_planes != buf->n_planes) {
		char format_str[ADF_FORMAT_STR_SIZE];
		adf_format_str(buf->format, format_str);
		dev_err(&dev->base.dev, "%u planes expected for format %s but %u planes provided\n",
			num_planes, format_str, buf->n_planes);
		return -EINVAL;
	}

	if (buf->w == 0 || buf->w % tile_w) {
		dev_err(&dev->base.dev, "bad buffer width %u\n", buf->w);
		return -EINVAL;
	}

	if (buf->h == 0 || buf->h % tile_h) {
		dev_err(&dev->base.dev, "bad buffer height %u\n", buf->h);
		return -EINVAL;
	}

	for (i = 0; i < num_planes; i++) {
		u32 width = buf->w / tile_w;
		u32 height = buf->h / tile_h;

		if (buf->pitch[i] < (u64) width * cpt[i]) {
			dev_err(&dev->base.dev, "plane %u pitch is shorter than buffer width (pitch = %u, tile width = %u, cpt = %u)\n",
				i, buf->pitch[i], width, cpt[i]);
			return -EINVAL;
		}

		if ((u64) ((height - 1) * buf->pitch[i]) + (width * cpt[i]) +
				buf->offset[i] > buf->dma_bufs[i]->size) {
			dev_err(&dev->base.dev, "plane %u buffer too small (tile height = %u, tile width = %u, pitch = %u, offset = %u, size = %zu)\n",
				i, height, width, buf->pitch[i],
				buf->offset[i], buf->dma_bufs[i]->size);
			return -EINVAL;
		}
	}

	return 0;
}

/*
 * This is a copy of adf_format_validate_yuv which removes the call to
 * adf_format_plane_cpp to allow its use for custom pixel formats.
 * This appears to be the intended original behaviour, as indicated by this
 * comment:
 *
 * "adf_format_validate_yuv() is intended to be called as a helper from @dev's
 * validate_custom_format() op."
 *
 * and otherwise the cpp argument is redundant.
 */
static int malidp_adf_format_validate_yuv(struct adf_device *dev,
		struct adf_buffer *buf,	u8 num_planes, u8 hsub, u8 vsub, u8 cpp[])
{
	u8 i;

	if (num_planes != buf->n_planes) {
		char format_str[ADF_FORMAT_STR_SIZE];
		adf_format_str(buf->format, format_str);
		dev_err(&dev->base.dev, "%u planes expected for format %s but %u planes provided\n",
				num_planes, format_str, buf->n_planes);
		return -EINVAL;
	}

	if (buf->w == 0 || buf->w % hsub) {
		dev_err(&dev->base.dev, "bad buffer width %u\n", buf->w);
		return -EINVAL;
	}

	if (buf->h == 0 || buf->h % vsub) {
		dev_err(&dev->base.dev, "bad buffer height %u\n", buf->h);
		return -EINVAL;
	}

	for (i = 0; i < num_planes; i++) {
		u32 width = buf->w / (i != 0 ? hsub : 1);
		u32 height = buf->h / (i != 0 ? vsub : 1);

		if (buf->pitch[i] < (u64) width * cpp[i]) {
			dev_err(&dev->base.dev, "plane %u pitch is shorter than buffer width (pitch = %u, width = %u, bpp = %u)\n",
					i, buf->pitch[i], width, cpp[i] * 8);
			return -EINVAL;
		}

		if ((u64) ((height - 1) * buf->pitch[i]) + (width * cpp[i]) +
				buf->offset[i] > buf->dma_bufs[i]->size) {
			dev_err(&dev->base.dev, "plane %u buffer too small (height = %u, width = %u, pitch = %u, offset = %u, size = %zu)\n",
					i, height, width, buf->pitch[i],
					buf->offset[i], buf->dma_bufs[i]->size);
			return -EINVAL;
		}
	}

	return 0;
}

int malidp_adf_format_validate(struct adf_device *dev,
		struct adf_buffer *buf)
{
	struct malidp_device *dp_dev = to_malidp_device(dev);
	u8 cpp[2];

	dev_dbg(dp_dev->device, "%s", __func__);

	switch (buf->format) {
	case MALIDP_FORMAT_XYUV:
	case MALIDP_FORMAT_VYU30:
		cpp[0] = 4;
		return malidp_adf_format_validate_yuv(dev, buf, 1, 1, 1, cpp);
	case MALIDP_FORMAT_Y0L2:
		cpp[0] = 8;
		return malidp_adf_format_validate_tiled(dev, buf, 1, 2, 2, cpp);
	case MALIDP_FORMAT_P010:
		cpp[0] = 2;
		cpp[1] = 4;
		return malidp_adf_format_validate_yuv(dev, buf, 2, 2, 2, cpp);
	case MALIDP_FORMAT_NV12AFBC:
	case MALIDP_FORMAT_NV16AFBC:
	case MALIDP_FORMAT_YUV10_420AFBC:
		/*
		 * We have to pass this validation because the malidp_buffer
		 * is not available. A proper validation will be done by
		 * malidp_adf_validate
		 */
		return 0;
	default:
		return -EINVAL;
	}
}

/*
 * Returns bits-per-pixel for AFBC compressed formats.
 *
 * This function returns the number of bits actually required by AFBC in the
 * "worst" case (uncompressed block). It's possible that the earlier validation
 * stage has already enforced a larger size requirement than this.
 */
static int malidp_adf_format_afbc_bpp(u32 format)
{
	switch (format) {
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
		return 32;
	case MALIDP_FORMAT_VYU30:
		return 30;
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
	case MALIDP_FORMAT_XYUV:
		return 24;
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_UYVY:
	case MALIDP_FORMAT_NV16AFBC:
		return 16;
	case MALIDP_FORMAT_YUV10_420AFBC:
		return 15;
	case MALIDP_FORMAT_NV12AFBC:
		return 12;
	default:
		return 0;
	}
}

bool malidp_adf_format_is_afbc_only(u32 format)
{
	switch (format) {
	case MALIDP_FORMAT_NV12AFBC:
	case MALIDP_FORMAT_NV16AFBC:
	case MALIDP_FORMAT_YUV10_420AFBC:
		return true;
	default:
		return false;
	}
}

int malidp_adf_format_afbc_validate(struct adf_device *dev,
		struct adf_buffer *adf_buf,
		struct malidp_buffer_config *dp_buf)
{
	struct malidp_device *dp_dev = to_malidp_device(dev);
	u32 n_superblocks;
	u32 total_w, total_h;
	u32 max_afbc_size, max_superblock_size;
	u32 format;
	u8 hss, vss;
	u8 bpp;

	if (adf_buf->n_planes != 1) {
		dev_err(dp_dev->device, "%s : AFBC buffers require 1 plane",
				__func__);
		return -EINVAL;
	}

	total_w = adf_buf->w + dp_buf->afbc_crop_l + dp_buf->afbc_crop_r;
	total_h = adf_buf->h + dp_buf->afbc_crop_t + dp_buf->afbc_crop_b;
	if ((total_w % 16) || (total_h % 16)) {
		dev_err(dp_dev->device, "%s : AFBC buffers must be aligned to 16 pixels",
				__func__);
		return -EINVAL;
	}

	switch (adf_buf->format) {
	case MALIDP_FORMAT_NV12AFBC:
		format = DRM_FORMAT_NV12;
		break;
	case MALIDP_FORMAT_NV16AFBC:
		format = DRM_FORMAT_NV16;
		break;
	case MALIDP_FORMAT_YUV10_420AFBC:
		format = DRM_FORMAT_YUV420;
		break;
	default:
		format = adf_buf->format;
	}
	hss = adf_format_horz_chroma_subsampling(format);
	vss = adf_format_vert_chroma_subsampling(format);
	if ((dp_buf->afbc_crop_l % hss) ||
		(dp_buf->afbc_crop_r % hss) ||
		(dp_buf->afbc_crop_t % vss) ||
		(dp_buf->afbc_crop_b % vss)) {
		dev_err(dp_dev->device, "%s : AFBC cropping not compatible with subsampling",
				__func__);
		return -EINVAL;
	}

	bpp = malidp_adf_format_afbc_bpp(adf_buf->format);
	if (!bpp) {
		char format_buf[ADF_FORMAT_STR_SIZE];
		adf_format_str(adf_buf->format, format_buf);
		dev_err(dp_dev->device, "%s : pixel format %s not supported for AFBC",
				__func__, format_buf);
		return -EINVAL;
	}

	n_superblocks = (total_w / AFBC_SUPERBLOCK_SIZE) *
		(total_h / AFBC_SUPERBLOCK_SIZE);

	BUG_ON((bpp * total_w * total_h) % 8);

	/* In sparse mode superblocks are 128-byte aligned */
	max_superblock_size = ALIGN(bpp *
		(AFBC_SUPERBLOCK_SIZE * AFBC_SUPERBLOCK_SIZE >> 3), 128);

	/* Space for the header and 64-byte alignment for body buffer */
	max_afbc_size = ALIGN(n_superblocks * AFBC_HEADER_SIZE, 64);
	/* Space for the body buffer */
	max_afbc_size += max_superblock_size * n_superblocks;

	if (max_afbc_size > (adf_buf->dma_bufs[0]->size - adf_buf->offset[0])) {
		dev_err(dp_dev->device, "%s : buffer size (%zu) with offset %i too small for %ix%i",
			__func__, adf_buf->dma_bufs[0]->size,
			adf_buf->offset[0], total_w, total_h);
		return -EINVAL;
	}

	return 0;
}
