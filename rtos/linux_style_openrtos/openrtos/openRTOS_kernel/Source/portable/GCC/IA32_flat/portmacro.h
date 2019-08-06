/*
	OpenRTOS V8.2.3  Copyright (C) 2015 Real Time Engineers Ltd.

	This file is part of the OpenRTOS product.

	OpenRTOS is distributed exclusively by WITTENSTEIN high integrity systems,
	and is subject to the terms of the License granted to your organization,
	including its warranties and limitations on use and distribution. It cannot be
	copied or reproduced in any way except as permitted by the License.

	Licenses authorize use by processor, compiler, business unit, and product.
	
	WITTENSTEIN high integrity systems is a trading name of WITTENSTEIN
	aerospace & simulation ltd, Registered Office: Brown's Court, Long Ashton
	Business Park, Yanley Lane, Long Ashton, Bristol, BS41 9LB, UK.
	Tel: +44 (0) 1275 395 600, fax: +44 (0) 1275 393 630.
	E-mail: info@HighIntegritySystems.com

	http://www.HighIntegritySystems.com
*/

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
	extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the given hardware
 * and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	uint32_t
#define portBASE_TYPE	long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

typedef uint32_t TickType_t;
#define portMAX_DELAY ( ( TickType_t ) 0xffffffffUL )

/*-----------------------------------------------------------*/

/* Hardware specifics. */
#define portSTACK_GROWTH			( -1 )
#define portTICK_PERIOD_MS			( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT			4 //Brill

/*-----------------------------------------------------------*/

/* Task utilities. */

/* The interrupt priority (for vectors 16 to 255) is determined using vector/16.
The quotient is rounded to the nearest integer with 1 being the lowest priority
and 15 is the highest.  Therefore the following two interrupts are at the lowest
priority.  *NOTE 1* If the yield vector is changed then it must also be changed
in the portYIELD_INTERRUPT definition immediately below. */
#define portAPIC_TIMER_INT_VECTOR 		( 0x25 )
#define portAPIC_YIELD_INT_VECTOR 		( 0x24 )

/* Build yield interrupt instruction. */
#define portYIELD_INTERRUPT "int $0x24"

/* APIC register addresses. */
#define portAPIC_EOI					(0xFEE000B0UL )

/* APIC bit definitions. */
#define portAPIC_ENABLE_BIT				( 1UL << 8UL )
#define portAPIC_TIMER_PERIODIC 		( 1UL << 17UL )
#define portAPIC_DISABLE 				( 1UL << 16UL )
#define portAPIC_NMI 					( 4 << 8)
#define portAPIC_DIV_16 				( 0x03 )

/* Define local API register addresses. */
#define portAPIC_ID_REGISTER			( configAPIC_BASE + 0x20UL  )
#define portAPIC_SPURIOUS_INT			( configAPIC_BASE + 0xF0UL  )
#define portAPIC_LVT_TIMER				( configAPIC_BASE + 0x320UL )
#define portAPIC_TIMER_INITIAL_COUNT	( configAPIC_BASE + 0x380UL )
#define portAPIC_TIMER_CURRENT_COUNT	( configAPIC_BASE + 0x390UL )
#define portAPIC_TASK_PRIORITY			( configAPIC_BASE + 0x80UL  )
#define portAPIC_LVT_ERROR				( configAPIC_BASE + 0x370UL )
#define portAPIC_ERROR_STATUS			( configAPIC_BASE + 0x280UL )
#define portAPIC_LDR	 				( configAPIC_BASE + 0xD0UL  )
#define portAPIC_TMRDIV 				( configAPIC_BASE + 0x3E0UL )
#define portAPIC_LVT_PERF 				( configAPIC_BASE + 0x340UL )
#define portAPIC_LVT_LINT0 				( configAPIC_BASE + 0x350UL )
#define portAPIC_LVT_LINT1 				( configAPIC_BASE + 0x360UL )

/* Don't yield if inside a critical section - instead hold the yield pending
so it is performed when the critical section is exited. */
#define portYIELD() 								\
{													\
extern volatile uint32_t ulCriticalNesting;			\
extern volatile uint32_t ulPortYieldPending;		\
	if( ulCriticalNesting != 0 )					\
	{												\
		ulPortYieldPending = pdTRUE;				\
	}												\
	else											\
	{												\
		__asm volatile( portYIELD_INTERRUPT );		\
	}												\
}

/* Called at the end of an ISR that can cause a context switch - pend a yield if
xSwithcRequired is not false. */
#define portEND_SWITCHING_ISR( xSwitchRequired )	\
{													\
extern volatile uint32_t ulPortYieldPending;		\
	if( xSwitchRequired != pdFALSE )				\
	{												\
		ulPortYieldPending = 1; 				 	\
	}												\
}

/* Same as portEND_SWITCHING_ISR() - take your pick which name to use. */
#define portYIELD_FROM_ISR( x ) portEND_SWITCHING_ISR( x )

/*-----------------------------------------------------------
 * Critical section control
 *----------------------------------------------------------*/

/* Critical sections for use in interrupts. */
#define portSET_INTERRUPT_MASK_FROM_ISR()		ulPortSetInterruptMask()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)	vPortClearInterruptMask( x )

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
extern uint32_t ulPortSetInterruptMask( void );
extern void vPortClearInterruptMask( uint32_t ulNewMaskValue );

/* These macros do not globally disable/enable interrupts.  They do mask off
interrupts that have a priority below configMAX_API_CALL_INTERRUPT_PRIORITY. */
#define portENTER_CRITICAL()		vPortEnterCritical()
#define portEXIT_CRITICAL()			vPortExitCritical()
#define portDISABLE_INTERRUPTS()	__asm volatile( "cli" )
#define portENABLE_INTERRUPTS()		__asm volatile( "sti" )

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not required for this port but included in case common demo code that uses these
macros is used. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )	void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )	void vFunction( void *pvParameters )

/* Architecture specific optimisations. */
#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

	/* Store/clear the ready priorities in a bit map. */
	#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )	\
		__asm volatile(	"bsr %1, %0\n\t" 									\
						:"=r"(uxTopPriority) : "rm"(uxReadyPriorities) : "cc" )

	#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
	#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

#define portNOP() __asm volatile( "NOP" )

/*-----------------------------------------------------------
 * Misc
 *----------------------------------------------------------*/

#define portNUM_VECTORS		256
#define portMAX_PRIORITY	15
typedef void ( *ISR_Handler_t ) ( void );

/* FPU context size if FSAVE is used. */
#define portFPU_CONTEXT_SIZE_BYTES 		108

#ifndef configSUPPORT_FPU
	#warning configSUPPORT_FPU was not defined in FreeRTOSConfig.h and has been defaulted to 0
	#define configSUPPORT_FPU 0
#endif

#ifndef ISH_CONFIG_MAX_FPU_TASKS
	#define ISH_CONFIG_MAX_FPU_TASKS 0
#endif

#if configSUPPORT_FPU == 1
	void vPortTaskUsesFPU( void );
	#define portTASK_USES_FLOATING_POINT() vPortTaskUsesFPU()
#endif

/* See the comments under the configUSE_COMMON_INTERRUPT_ENTRY_POINT definition
below. */
BaseType_t xPortRegisterCInterruptHandler( ISR_Handler_t pxHandler, uint32_t ulVectorNumber );
BaseType_t xPortInstallInterruptHandler( ISR_Handler_t pxHandler, uint32_t ulVectorNumber );

#ifndef configAPIC_BASE
	/* configAPIC_BASE_ADDRESS sets the base address of the local APIC.  It can
	be overridden in FreeRTOSConfig.h should it not be constant. */
	#define configAPIC_BASE	0xFEE00000UL
#endif

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
	/* The FreeRTOS scheduling algorithm selects the task that will enter the
	Running state.  configUSE_PORT_OPTIMISED_TASK_SELECTION is used to set how
	that is done.

	If configUSE_PORT_OPTIMISED_TASK_SELECTION is set to 0 then the task to
	enter the Running state is selected using a portable algorithm written in
	C.  This is the slowest method, but the algorithm does not restrict the
	maximum number of unique RTOS task priorities that are available.

	If configUSE_PORT_OPTIMISED_TASK_SELECTION is set to 1 then the task to
	enter the Running state is selected using a single assembly instruction.
	This is the fastest method, but restricts the maximum number of unique RTOS
	task priorities to 32 (the same task priority can be assigned to any number
	of RTOS	tasks). */
	#warning configUSE_PORT_OPTIMISED_TASK_SELECTION was not defined in FreeRTOSConfig.h and has been defaulted to 1
	#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#ifndef configUSE_COMMON_INTERRUPT_ENTRY_POINT
	/* There are two ways of implementing interrupt handlers:

		1) As standard C functions -

		This method can only be used if configUSE_COMMON_INTERRUPT_ENTRY_POINT
		is set to 1.  The C function is installed using
		xPortRegisterCInterruptHandler().

		This is the simplest of the two methods but incurs a slightly longer
		interrupt entry time.

		2) By using an assembly stub that wraps the handler in the FreeRTOS
		   portFREERTOS_INTERRUPT_ENTRY and portFREERTOS_INTERRUPT_EXIT macros.

		This method can always be used.  It is slightly more complex than
		method 1 but benefits from a faster interrupt entry time. */
	#warning config_USE_COMMON_INTERRUPT_ENTRY_POINT was not defined in FreeRTOSConfig.h and has been defaulted to 1.
	#define configUSE_COMMON_INTERRUPT_ENTRY_POINT	1
#endif

#ifndef configISR_STACK_SIZE
	/* Interrupt entry code will switch the stack in use to a dedicated system 
	stack.

	configISR_STACK_SIZE defines the number of 32-bit values that can be stored
	on the system stack, and must be large enough to hold a potentially nested
	interrupt stack frame. */

	#error configISE_STACK_SIZE was not defined in FreeRTOSConfig.h.
#endif

#ifndef configMAX_API_CALL_INTERRUPT_PRIORITY
	/* Interrupt safe FreeRTOS functions (those that end in "FromISR" must not
	be called from an interrupt that has a priority above that set by
	configMAX_API_CALL_INTERRUPT_PRIORITY.  */
	#warning configMAX_API_CALL_INTERRUPT_PRIORITY was not defined in FreeRTOSConfig.h and has been defaulted to 10
	#define configMAX_API_CALL_INTERRUPT_PRIORITY 10
#endif


/* The value written to the task priority register to raise the interrupt mask
to the maximum from which FreeRTOS API calls can be made. */
#define portAPIC_PRIORITY_SHIFT		( 4UL )
#define portAPIC_MAX_SUB_PRIORITY	( 0x0fUL )
#define portMAX_API_CALL_PRIORITY		( ( configMAX_API_CALL_INTERRUPT_PRIORITY << portAPIC_PRIORITY_SHIFT ) | portAPIC_MAX_SUB_PRIORITY )

/* Asserts if interrupt safe FreeRTOS functions are called from a priority
above the max system call interrupt priority. */
#define portAPIC_PROCESSOR_PRIORITY	( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0xA0UL  ) ) )
#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID() configASSERT( ( portAPIC_PROCESSOR_PRIORITY ) <= ( portMAX_API_CALL_PRIORITY ) )

/* A structure used to map the various fields of an IDT entry into separate
structure members. */
struct IDTEntry
{
    uint16_t usISRLow;              /* Low 16 bits of handler address. */
    uint16_t usSegmentSelector;     /* Flat model means this is not changed. */
    uint8_t ucZero;                 /* Must be set to zero. */
    uint8_t ucFlags;                /* Flags for this entry. */
    uint16_t usISRHigh;             /* High 16 bits of handler address. */
} __attribute__( ( packed ) );
typedef struct IDTEntry IDTEntry_t;

/* Use to pass the location of the IDT to the CPU. */
struct IDTPointer
{
   uint16_t usTableLimit;
   uint32_t ulTableBase;                /* The address of the first entry in xInterruptDescriptorTable. */
} __attribute__( ( __packed__ ) );
typedef struct IDTPointer IDTPointer_t;

/* Default flags setting for entries in the IDT.
 * Flags format: (P, 6-5: DPL, 0, D, 1, 1, 0)
 *
 * P = Segment Present flag
 * DPL = Descriptor Privilege Level
 * D = Size of gate: 1 - 32 bits; 0 - 16 bits
 */
#define portIDT_FLAGS                   ( 0x8E )


#ifdef __cplusplus
	} /* extern C */
#endif

#endif /* PORTMACRO_H */

