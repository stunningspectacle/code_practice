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

#ifndef _TRC_RECORDER_WRAPPER_H
#define _TRC_RECORDER_WRAPPER_H

#include <ish_config.h>

#if (ISH_CONFIG_SUPPORT_RTOS_TRACE_RECORDER == 1)
/* Integrates the Tracealyzer recorder library */
#include "trcRecorder.h"

/* redicrect to tracelayzer recodder library's definition */
typedef traceHandle RTOS_TRACE_HANDLE ;
typedef traceString RTOS_TRACE_STRING ;

#define TRACE_START                         TRC_START

#define RTOS_TRACE_ENABLE(x)                vTraceEnable(x)
#define RTOS_TRACE_REGISTER_STRING(x)       xTraceRegisterString(x)

#define RTOS_TRACE_PRINT(chn, ...)          vTracePrint((chn), __VA_ARGS__) 
#define RTOS_TRACE_PRINT_FMT(chn, ...)      vTracePrintF((chn), __VA_ARGS__)

#define RTOS_TRACE_ISR_BEGIN(x)             vTraceStoreISRBegin(x)
#define RTOS_TRACE_ISR_END(x)               vTraceStoreISREnd(x) 

#define RTOS_TRACE_CREATE_ISR(a, b)         xTraceSetISRProperties(a, b) 

#define RTOS_TRACE_START()                  vTraceStart()
#define RTOS_TRACE_STOP()                   vTraceStop()
#define RTOS_TRACE_COPY_TRACE_BUFFER(a,b)   vTraceCopyTraceBuffer(a, b)

/* common resources definition */
#define PRIO_ISR_I2C0      1
#define PRIO_ISR_I2C1      2 
#define PRIO_ISR_I2C2      3 
#define PRIO_ISR_GPIO      4

#define PRIO_ISR_IPC       5 
#define PRIO_ISR_DMA       6 

#define PRIO_ISR_UART0     7 
#define PRIO_ISR_UART1     8

#define PRIO_ISR_HPET0     9 
#define PRIO_ISR_HPET12    10 
#define PRIO_ISR_WDT       11 

extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_i2c0_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_i2c1_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_i2c2_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_gpio_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_ipc_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_dma_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_uart0_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_uart1_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_wdt_handle ;

// extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_hpet0_handle ;
extern __pinned_kernel_data__ RTOS_TRACE_HANDLE tr_hpet12_handle ;

// string channel
extern __pinned_kernel_data__ RTOS_TRACE_STRING tr_i2c_channel ;

void create_rtos_trace_resources(void); 

#else

#define TRACE_START                (1) 

#define RTOS_TRACE_ENABLE(x)
#define RTOS_TRACE_REGISTER_STRING(x)

#define RTOS_TRACE_PRINT(chn, ...)
#define RTOS_TRACE_PRINT_FMT(chn, ...)

#define RTOS_TRACE_ISR_BEGIN(x)
#define RTOS_TRACE_ISR_END(x)

#define RTOS_TRACE_CREATE_ISR(a, b)

#define RTOS_TRACE_START()
#define RTOS_TRACE_STOP()

#define RTOS_TRACE_COPY_TRACE_BUFFER(a,b)

#define create_rtos_trace_resources()

#endif

#endif //_TRC_RECORDER_WRAPPER_H
