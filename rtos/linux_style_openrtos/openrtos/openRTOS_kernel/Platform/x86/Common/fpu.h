/*++
 INTEL CONFIDENTIAL
 Copyright (c) 2012 - 2015 Intel Corporation. All Rights Reserved.

 The source code contained or described herein and all documents related
 to the source code ("Material") are owned by Intel Corporation or its
 suppliers or licensors. Title to the Material remains with Intel Corporation
 or its suppliers and licensors. The Material contains trade secrets and
 proprietary and confidential information of Intel or its suppliers and
 licensors. The Material is protected by worldwide copyright and trade secret
 laws and treaty provisions. No part of the Material may be used, copied,
 reproduced, modified, published, uploaded, posted, transmitted, distributed,
 or disclosed in any way without Intel's prior express written permission.

 No license under any patent, copyright, trade secret or other intellectual
 property right is granted to or conferred upon you by disclosure or delivery
 of the Materials, either expressly, by implication, inducement, estoppel or
 otherwise. Any license under such intellectual property rights must be
 express and approved by Intel in writing.
 --*/

#ifndef _OPENRTOS_FPU_H
#define _OPENRTOS_FPU_H

#include <os_api.h>
#include <ish_config.h>

void fp_device_not_available_handler(EXC_FRAME *exc);

__pinned_kernel_bss__ static volatile int8_t currentNumberOfFpTasks = 0;
__pinned_kernel_bss__ uint8_t FPU_POOL[ISH_CONFIG_MAX_FPU_TASKS * portFPU_CONTEXT_SIZE_BYTES];


#endif //_OPENRTOS_FPU_H
