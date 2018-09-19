/*
 *
 * (C) COPYRIGHT 2013-2014 ARM Limited. All rights reserved.
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



#ifndef _MALIDP_ADF_FORMAT_H_
#define _MALIDP_ADF_FORMAT_H_

#include <video/adf.h>

int malidp_adf_format_validate(struct adf_device *dev,
		struct adf_buffer *buf);

int malidp_adf_format_afbc_validate(struct adf_device *dev,
		struct adf_buffer *adf_buf,
		struct malidp_buffer_config *dp_buf);

bool malidp_adf_format_is_afbc_only(u32 format);

#endif /* _MALIDP_ADF_FORMAT_H_ */
