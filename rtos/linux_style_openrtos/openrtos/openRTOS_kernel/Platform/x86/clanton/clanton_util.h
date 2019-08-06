/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2016 Intel Corporation. All Rights Reserved.

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

//---------------------------------------------------------------------
// MMIO read/write/set/clear/modify macros
//---------------------------------------------------------------------
#define mem_read(base, offset, size) ({ \
	volatile uint32_t a = (base) + (offset); \
	volatile uint64_t v; \
    switch (size) { \
    case 1: \
        v = (uint8_t)(*((uint8_t *)a)); \
        break; \
    case 2: \
        v = (uint16_t)(*((uint16_t *)a)); \
        break; \
    case 4: \
        v = (uint32_t)(*((uint32_t *)a)); \
        break; \
    case 8: \
        v = (uint64_t)(*((uint64_t *)a)); \
        break; \
    default: \
       v = 0; \
       break; \
    } \
    v; \
})

