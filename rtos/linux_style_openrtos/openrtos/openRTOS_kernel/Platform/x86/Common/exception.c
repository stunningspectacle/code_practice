/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2012 - 2014 Intel Corporation. All Rights Reserved.

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

#include "FreeRTOS.h"
#include "exception_support.h"
#include "limits.h"
#include "exception.h"
#include "FreeRTOSConfig.h"
#include "ish_sections.h"





/*
 * Set to true when vPortStartFirstTask() is called at the first time.
 * Exception handlers use it to know if stack switch to system stack is needed.
 */
volatile uint8_t initDone = pdFALSE;


/*
 * A pointer to the IDT defined in port.c
 */
extern IDTEntry_t xInterruptDescriptorTable[portNUM_VECTORS];



__init_flow_code__ void setExceptionGate( uint8_t vector, ISR_Handler_t pxHandlerFunction)
{
uint32_t ulBase = ( uint32_t ) pxHandlerFunction;

    xInterruptDescriptorTable[ vector ].usISRLow = ( uint16_t ) ( ulBase & USHRT_MAX );
    xInterruptDescriptorTable[ vector ].usISRHigh = ( uint16_t ) ( ( ulBase >> 16UL ) & USHRT_MAX );
    xInterruptDescriptorTable[ vector ].usSegmentSelector = 0x8; // kernel CS
    xInterruptDescriptorTable[ vector ].ucZero = 0;
    xInterruptDescriptorTable[ vector ].ucFlags = portIDT_FLAGS;
}

__init_flow_code__ void setupInitialIDT()
{
IDTPointer_t xIDT;

#if ISH_CONFIG_SUPPORT_PAGING
extern void vPortPageFaultHandler( void );
    setExceptionGate( ( uint8_t ) EXC_PAGE_FAULT, vPortPageFaultHandler);
#endif

    setExceptionGate(EXC_DEVIDE_ERROR, vPortDivideErrorWrapper);
    setExceptionGate(EXC_NMI_INTERRUPT, vPortNmiIntteruptWrapper);
    setExceptionGate(EXC_OVERFLOW, vPortOverflowWrapper);
    setExceptionGate(EXC_BOUND_RANGE_EXCEED, vPortBoundRangeExceededWrapper);
    setExceptionGate(EXC_INVALID_OPCODE, vPortInvalidOpcodeWrapper);
    setExceptionGate(EXC_DEVICE_NOT_AVAILABLE, vPortDeviceNotAvailableWrapper);
    setExceptionGate(EXC_DOUBLE_FAULT, vPortDoubleFaultWrapper);
    setExceptionGate(EXC_COPROCESSOR_SEGMENT_OVERRUN, vPortCoprocessorSegmentOverrunWrapper);
    setExceptionGate(EXC_INVALID_TSS, vPortInvalidTssWrapper);
    setExceptionGate(EXC_SEGMENT_NOT_PRESENT, vPortSegmentNotPresentWrapper);
    setExceptionGate(EXC_STACK_FAULT, vPortStackFaultWrapper);
    setExceptionGate(EXC_GENERAL_PROTECTION, vPortGeneralProtectionWrapper);
    setExceptionGate(EXC_FPU_FLOATING_POINT_ERROR, vPortFpuFloatingPointWrapper);
    setExceptionGate(EXC_ALIGNMENT_CHECK, vPortAlignmentCheckWrapper);
    setExceptionGate(EXC_MACHINE_CHECK, vPortMachineCheckWrapper);
    setExceptionGate(EXC_SIMD_FLOATING_POINT, vPortSimdFloatingPointWrapper);

    /* Set IDT address. */
    xIDT.ulTableBase = ( uint32_t ) xInterruptDescriptorTable;
    xIDT.usTableLimit = sizeof( xInterruptDescriptorTable ) - 1;

    /* Set IDT in CPU. */
    __asm volatile( "lidt %0" :: "m" (xIDT) );
}

#if ISH_CONFIG_EXCEPTION_LOGGING

__pinned_kernel_code__ void get_exc_call_stack(uint32_t eip, uint32_t ebp, exception_info_t *exc_info)
{
extern volatile uint8_t initDone;

	int *ret_addr = 0;
	int *next_frame = 0;
	int i = 0;

	//start extracting call stack
	ret_addr = (int*)eip;
	next_frame = (int*)ebp;

	//walk through call stack
	for(i=0; (next_frame !=0) && (i < STACK_DEPTH_MAX) && ((uint32_t)next_frame != 0x00) && (initDone != 0); i++)
	{
		exc_info->call_stack[i] = (uint32_t)ret_addr;
		ret_addr = (int*)*(next_frame + 1);
		next_frame = (int*)*next_frame;
	}

	//log first call and put 0xffffffff (if there is a space) at the end for debugger
	if (i < STACK_DEPTH_MAX)
	{
		exc_info->call_stack[i++] = (uint32_t)ret_addr;
		if (i < STACK_DEPTH_MAX)
		{
			exc_info->call_stack[i] = 0xffffffff;
		}
	}


}

#endif

void callstack_dump(void)
{
    unsigned int ebp;
    unsigned int *pEBP;
    unsigned int callstack[STACK_DEPTH_MAX];
    int depth = 0;

    asm ("movl %%ebp, %0" : "=r" (ebp));

    pEBP = (unsigned int *)ebp;

    for (int i=0; (i<STACK_DEPTH_MAX) && (NULL != pEBP); i++) {
        callstack[i] = (unsigned int)*(pEBP+1);
        pEBP = (unsigned int *)*pEBP;
        depth++;
    }

    printk("Callstack:\n");
    for (int i=0; i<depth; i++) {
        printk("\t%08X\n", callstack[i]);
    }
}

