/*++
  INTEL CONFIDENTIAL
  Copyright (c) 2012 - 2017 Intel Corporation. All Rights Reserved.

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

#include "rtos_trc_recorder_wrapper.h"

#if (ISH_CONFIG_SUPPORT_RTOS_TRACE_RECORDER == 1)

__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_i2c0_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_i2c1_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_i2c2_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_gpio_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_ipc_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_dma_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_uart0_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_uart1_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_uart2_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_wdt_handle = 0 ;

//__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_hpet0_handle = 0 ;
__pinned_kernel_data__ RTOS_TRACE_HANDLE tr_hpet12_handle = 0 ;

// string channel
__pinned_kernel_data__ RTOS_TRACE_STRING tr_i2c_channel = 0 ;

void create_rtos_trace_resources(void) 
{
    tr_i2c0_handle = RTOS_TRACE_CREATE_ISR("ISR_I2C0", PRIO_ISR_I2C0);
    tr_i2c1_handle = RTOS_TRACE_CREATE_ISR("ISR_I2C1", PRIO_ISR_I2C1);
    tr_i2c2_handle = RTOS_TRACE_CREATE_ISR("ISR_I2C2", PRIO_ISR_I2C2);

    tr_gpio_handle = RTOS_TRACE_CREATE_ISR("ISR_GPIO", PRIO_ISR_GPIO);
    tr_ipc_handle  =  RTOS_TRACE_CREATE_ISR("ISR_IPC",  PRIO_ISR_IPC);
    tr_dma_handle  =  RTOS_TRACE_CREATE_ISR("ISR_DMA",  PRIO_ISR_DMA);

    tr_uart0_handle =  RTOS_TRACE_CREATE_ISR("ISR_UART0",  PRIO_ISR_UART0);
    tr_uart1_handle =  RTOS_TRACE_CREATE_ISR("ISR_UART1",  PRIO_ISR_UART1);

    //tr_hpet0_handle =  RTOS_TRACE_CREATE_ISR("ISR_HPET0",  PRIO_ISR_HPET0);
    tr_hpet12_handle =  RTOS_TRACE_CREATE_ISR("ISR_HPET12",  PRIO_ISR_HPET12);

    tr_wdt_handle =  RTOS_TRACE_CREATE_ISR("ISR_WDT",  PRIO_ISR_WDT);

    tr_i2c_channel = RTOS_TRACE_REGISTER_STRING("i2c");
}

#endif
