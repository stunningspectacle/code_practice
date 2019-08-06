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

/* Standard includes. */
#include <limits.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <ia32_interrupts.h>
#include <ish_hw.h>
#include <ish_sections.h>
#include <reg_rw.h>

#if ( configUSE_PORT_OPTIMISED_TASK_SELECTION == 1 )
	/* Check the configuration. */
	#if( configMAX_PRIORITIES > 32 )
		#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
	#endif
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

#if( configISR_STACK_SIZE < ( configMINIMAL_STACK_SIZE * 2 ) )
	#warning configISR_STACK_SIZE is probably too small!
#endif /* ( configISR_STACK_SIZE < configMINIMAL_STACK_SIZE * 2 ) */

#if( ( configMAX_API_CALL_INTERRUPT_PRIORITY > portMAX_PRIORITY ) || ( configMAX_API_CALL_INTERRUPT_PRIORITY < 2 ) )
	#error configMAX_API_CALL_INTERRUPT_PRIORITY must be between 2 and 15
#endif

/* A critical section is exited when the critical section nesting count reaches
this value. */
#define portNO_CRITICAL_NESTING			( ( uint32_t ) 0 )

/* Tasks are not created with a floating point context, but can be given a
floating point context after they have been created.  A variable is stored as
part of the tasks context that holds portNO_FLOATING_POINT_CONTEXT if the task
does not have an FPU context, or any other value if the task does have an FPU
context. */
#define portNO_FLOATING_POINT_CONTEXT	( ( StackType_t ) 0 )

/* Only the IF bit is set so tasks start with interrupts enabled. */
#define portINITIAL_EFLAGS				( 0x200UL )

/* Error interrupts are at the highest priority vectors. */
#define portAPIC_LVT_ERROR_VECTOR 		( 0xfe )
#define portAPIC_SPURIOUS_INT_VECTOR 	( 0xff )

/* EFLAGS bits. */
#define portEFLAGS_IF					( 0x200UL )

/* The expected size of each entry in the IDT.  Used to check structure packing
 is set correctly. */
#define portEXPECTED_IDT_ENTRY_SIZE		8

/* This is the lowest possible ISR vector available to application code. */
#define portAPIC_MIN_ALLOWABLE_VECTOR	( 0x20 )

/* If configASSERT() is defined then the system stack is filled with this value
to allow for a crude stack overflow check. */
#define portSTACK_WORD					( 0xecececec )
/*-----------------------------------------------------------*/

/*
 * Starts the first task executing.
 */
extern void vPortStartFirstTask( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
void prvTaskExitError( void );

/*
 * Complete one descriptor in the IDT.
 */
static void prvSetInterruptGate( uint8_t ucNumber, ISR_Handler_t pxHandlerFunction, uint8_t ucFlags );

/*
 * The default handler installed in each IDT position.
 */
extern void vPortCentralInterruptWrapper( void );

/*
 * Handler for portYIELD().
 */
extern void vPortYieldCall( void );

/*
 * Configure the APIC to generate the RTOS tick.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Tick interrupt handler.
 */
extern void vPortTimerHandler( void );
extern void vPortWdtHandler(void);

/*
 * Check an interrupt vector is not too high, too low, in use by FreeRTOS, or
 * already in use by the application.
 */
static BaseType_t prvCheckValidityOfVectorNumber( uint32_t ulVectorNumber );


/*-----------------------------------------------------------*/

/* A variable is used to keep track of the critical section nesting.  This
variable must be initialised to a non zero value to ensure interrupts don't
inadvertently become unmasked before the scheduler starts. It is set to zero
before the first task starts executing. */
__pinned_kernel_data__ volatile uint32_t ulCriticalNesting = 9999UL;

/* The IDT itself. */
__pinned_kernel_data__ __attribute__ ( ( aligned( 32 ) ) ) IDTEntry_t xInterruptDescriptorTable[ portNUM_VECTORS ];

#if ( configUSE_COMMON_INTERRUPT_ENTRY_POINT == 1 )

	/* A table in which application defined interrupt handlers are stored.  These
	are called by the central interrupt handler if a common interrupt entry
	point it used. */
	static ISR_Handler_t xInterruptHandlerTable[ portNUM_VECTORS ] = { NULL };

#endif /* configUSE_COMMON_INTERRUPT_ENTRY_POINT */

/* The stack used by interrupt handlers. */
__stacks_section__("ulSystemStack") uint32_t ulSystemStack[ configISR_STACK_SIZE ] __attribute__((used))  = { 0 };

/* Don't use the very top of the system stack so the return address
appears as 0 if the debugger tries to unwind the stack. */
 __pinned_kernel_data__ volatile uint32_t ulTopOfSystemStack __attribute__((aligned(0x4))) = ( uint32_t ) &( ulSystemStack[ configISR_STACK_SIZE - sizeof(uint32_t) ] );

/* If a yield is requested from an interrupt or from a critical section then
the yield is not performed immediately, and ulPortYieldPending is set to pdTRUE
instead to indicate the yield should be performed at the end of the interrupt
when the critical section is exited. */
__pinned_kernel_data__ volatile uint32_t ulPortYieldPending __attribute__((used)) = pdFALSE;

/* Counts the interrupt nesting depth.  Used to know when to switch to the
interrupt/system stack and when to save/restore a complete context. */
__pinned_kernel_data__ volatile uint32_t ulInterruptNesting __attribute__((used)) = 0;


extern void printk( const char *fmt,	/* formatted string to output */  ...  );

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
uint32_t ulCodeSegment;

	/* Setup the initial stack as expected by the portFREERTOS_INTERRUPT_EXIT macro. */

	*pxTopOfStack = 0x00;
	pxTopOfStack--;
	*pxTopOfStack = 0x00;
	pxTopOfStack--;

	/* Parameters first. */
	*pxTopOfStack = ( StackType_t ) pvParameters;
	pxTopOfStack--;

	/* There is nothing to return to so assert if attempting to use the return
	address. */
	*pxTopOfStack = ( StackType_t ) prvTaskExitError;
	pxTopOfStack--;

	/* iret used to start the task pops up to here. */
	*pxTopOfStack = portINITIAL_EFLAGS;
	pxTopOfStack--;

	/* CS */
	__asm volatile( "movl %%cs, %0" : "=r" ( ulCodeSegment ) );
	*pxTopOfStack = ulCodeSegment;
	pxTopOfStack--;

	/* First instruction in the task. */
	*pxTopOfStack = ( StackType_t ) pxCode;
	pxTopOfStack--;

	/* General purpose registers as expected by a POPA instruction. */
	*pxTopOfStack = 0xEA;
	pxTopOfStack--;

	*pxTopOfStack = 0xEC;
	pxTopOfStack--;

	*pxTopOfStack = 0xED1; /* EDX */
	pxTopOfStack--;

	*pxTopOfStack = 0xEB1; /* EBX */
	pxTopOfStack--;

	/* Hole for ESP. */
	pxTopOfStack--;

	*pxTopOfStack = 0x00; /* EBP */
	pxTopOfStack--;

	*pxTopOfStack = 0xE5; /* ESI */
	pxTopOfStack--;

	*pxTopOfStack = 0xeeeeeeee; /* EDI */


	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvSetInterruptGate( uint8_t ucNumber, ISR_Handler_t pxHandlerFunction, uint8_t ucFlags )
{
uint16_t usCodeSegment;
uint32_t ulBase = ( uint32_t ) pxHandlerFunction;

	xInterruptDescriptorTable[ ucNumber ].usISRLow = ( uint16_t ) ( ulBase & USHRT_MAX );
	xInterruptDescriptorTable[ ucNumber ].usISRHigh = ( uint16_t ) ( ( ulBase >> 16UL ) & USHRT_MAX );

	/* When the flat model is used the CS will never change. */
	__asm volatile( "mov %%cs, %0" : "=r" ( usCodeSegment ) );
	xInterruptDescriptorTable[ ucNumber ].usSegmentSelector = usCodeSegment;
	xInterruptDescriptorTable[ ucNumber ].ucZero = 0;
	xInterruptDescriptorTable[ ucNumber ].ucFlags = ucFlags;
}
/*-----------------------------------------------------------*/

void vPortSetupIDT( void )
{
uint32_t ulNum;
IDTPointer_t xIDT;

	#if ( configUSE_COMMON_INTERRUPT_ENTRY_POINT == 1 )
	{
		for( ulNum = 0; ulNum < portNUM_VECTORS; ulNum++ )
		{
			/* If a handler has not already been installed on this vector. */
			if( ( xInterruptDescriptorTable[ ulNum ].usISRLow == 0x00 ) && ( xInterruptDescriptorTable[ ulNum ].usISRHigh == 0x00 ) )
			{
				prvSetInterruptGate( ( uint8_t ) ulNum, vPortCentralInterruptWrapper, portIDT_FLAGS );
			}
		}
	}
	#endif /* configUSE_COMMON_INTERRUPT_ENTRY_POINT */

	/* Set IDT address. */
	xIDT.ulTableBase = ( uint32_t ) xInterruptDescriptorTable;
	xIDT.usTableLimit = sizeof( xInterruptDescriptorTable ) - 1;

	/* Set IDT in CPU. */
	__asm volatile( "lidt %0" :: "m" (xIDT) );
}
/*-----------------------------------------------------------*/

void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).

	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
	configASSERT( ulCriticalNesting == ~0UL );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvSetupTimerInterrupt( void )
{
extern void vPortAPICErrorHandlerWrapper( void );
extern void vPortAPICSpuriousHandler( void );

	/* Initialise LAPIC to a well known state. */
	write32(portAPIC_LDR, 0xFFFFFFFF);
	write32(portAPIC_LDR,(read32(portAPIC_LDR) & 0x00FFFFFF ) | 0x00000001);
	write32(portAPIC_LVT_TIMER, portAPIC_DISABLE);
	write32(portAPIC_LVT_PERF, portAPIC_NMI);
	write32(portAPIC_LVT_LINT0, portAPIC_DISABLE);
	write32(portAPIC_LVT_LINT1, portAPIC_DISABLE);
	write32(portAPIC_TASK_PRIORITY, 0);


	/* Install API error handler. */
	prvSetInterruptGate( ( uint8_t ) portAPIC_LVT_ERROR_VECTOR, vPortAPICErrorHandlerWrapper, portIDT_FLAGS );

	/* Install Yield handler. */
	prvSetInterruptGate( ( uint8_t ) portAPIC_YIELD_INT_VECTOR, vPortYieldCall, portIDT_FLAGS );

	/* Install spurious interrupt vector. */
	prvSetInterruptGate( ( uint8_t ) portAPIC_SPURIOUS_INT_VECTOR, vPortAPICSpuriousHandler, portIDT_FLAGS );

	prvSetInterruptGate(WDT_VEC, vPortWdtHandler, portIDT_FLAGS);

	/* Enable the APIC, mapping the spurious interrupt at the same time. */
	write32(portAPIC_SPURIOUS_INT, portAPIC_SPURIOUS_INT_VECTOR | portAPIC_ENABLE_BIT);

	/* Set timer error vector. */
	write32(portAPIC_LVT_ERROR, portAPIC_LVT_ERROR_VECTOR);
}
/*-----------------------------------------------------------*/

void hpet_isr(void);
void hpet_init();
/* hpet timer should be initialized before the scheduler start */
void hpet_timer_init(void)
{
	xPortRegisterCInterruptHandler(hpet_isr,  HPET_TIMER0_VEC);
	hpet_init();
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
	/* Some versions of GCC require the -mno-ms-bitfields command line option
	for packing to work. */
	configASSERT( sizeof( struct IDTEntry ) == portEXPECTED_IDT_ENTRY_SIZE );
    
    //disable critical section logic so we don't enable interrupts during vPortExitCritical until we are ready to jump to first task
    ulCriticalNesting = 1;
    
	/* Initialise Interrupt Descriptor Table (IDT). */
	vPortSetupIDT();

	/* Initialise LAPIC and install system handlers. */
	prvSetupTimerInterrupt();

	hpet_timer_init();

    //enable critical sections again
    ulCriticalNesting = 0;
    
	/* Should not return from the following function as the scheduler will then
	be executing the tasks. */
	vPortStartFirstTask();

	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented in ports where there is nothing to return to.
	Artificially force an assert. */
	configASSERT( ulCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	if( ulCriticalNesting == 0 )
	{
		#if( configMAX_API_CALL_INTERRUPT_PRIORITY == portMAX_PRIORITY )
		{
			__asm volatile( "cli" );
		}
		#else
		{
			write32(portAPIC_TASK_PRIORITY, portMAX_API_CALL_PRIORITY);
			configASSERT( read32(portAPIC_TASK_PRIORITY) == portMAX_API_CALL_PRIORITY );
		}
		#endif
	}

	/* Now interrupts are disabled ulCriticalNesting can be accessed
	directly.  Increment ulCriticalNesting to keep a count of how many times
	portENTER_CRITICAL() has been called. */
	ulCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	if( ulCriticalNesting > portNO_CRITICAL_NESTING )
	{
		/* Decrement the nesting count as the critical section is being
		exited. */
		ulCriticalNesting--;

		/* If the nesting level has reached zero then all interrupt
		priorities must be re-enabled. */
		if( ulCriticalNesting == portNO_CRITICAL_NESTING )
		{
			/* Critical nesting has reached zero so all interrupt priorities
			should be unmasked. */
			#if( configMAX_API_CALL_INTERRUPT_PRIORITY == portMAX_PRIORITY )
			{
				__asm volatile( "sti" );
			}
			#else
			{
				write32(portAPIC_TASK_PRIORITY, 0);
			}
			#endif

			/* If a yield was pended from within the critical section then
			perform the yield now. */
			if( ulPortYieldPending != pdFALSE )
			{
				ulPortYieldPending = pdFALSE;
				__asm volatile( portYIELD_INTERRUPT );
			}


		}
	}
}
/*-----------------------------------------------------------*/

__pinned_kernel_code__ uint32_t ulPortSetInterruptMask( void )
{
volatile uint32_t ulOriginalMask = 0;

	/* Set mask to max syscall priority. */
	#if( configMAX_API_CALL_INTERRUPT_PRIORITY == portMAX_PRIORITY )
	{
		/* Return whether interrupts were already enabled or not.  Pop adjusts
		the stack first. */
		__asm volatile( "pushf		\t\n"
						"pop %0		\t\n"
						"cli			"
						: "=rm" (ulOriginalMask) :: "memory" );

		ulOriginalMask &= portEFLAGS_IF;
	}
	#else
	{
		/* Return original mask. */
		ulOriginalMask = read32(portAPIC_TASK_PRIORITY);
		write32(portAPIC_TASK_PRIORITY, portMAX_API_CALL_PRIORITY);
		configASSERT( read32(portAPIC_TASK_PRIORITY) == portMAX_API_CALL_PRIORITY );
	}
	#endif

	return ulOriginalMask;
}
/*-----------------------------------------------------------*/

__pinned_kernel_code__ void vPortClearInterruptMask( uint32_t ulNewMaskValue )
{
	// !!! THIS FUNCTION CONTAINS A BUG, DON'T USE IT. !!!
	#if( configMAX_API_CALL_INTERRUPT_PRIORITY == portMAX_PRIORITY )
	{
		if( ulNewMaskValue != pdFALSE )
		{
			// OpenRTOS BUG: We should restore eflags given by ulNewMaskValue rather than explicitly enable interrupt.
			__asm volatile( "sti" );
		}
	}
	#else
	{
		write32(portAPIC_TASK_PRIORITY, ulNewMaskValue);
		configASSERT( read32(portAPIC_TASK_PRIORITY) == ulNewMaskValue );
	}
	#endif
}
/*-----------------------------------------------------------*/

#if ( configSUPPORT_FPU == 1 )

	void vPortTaskUsesFPU( void )
	{
		//Empty function to keep OPENRTOS implementation. 
	}

#endif /* configSUPPORT_FPU */
/*-----------------------------------------------------------*/

#if ( configUSE_TICKLESS_IDLE != 0 )

	 void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
	 {
	extern void pm_execute_idle_flow(int32_t ticks);
		pm_execute_idle_flow(xExpectedIdleTime);
	}

#endif

void vPortAPICErrorHandler( void )
{
/* Variable to hold the APIC error status for viewing in the debugger. */
volatile uint32_t ulErrorStatus = 0;

	write32(portAPIC_ERROR_STATUS, 0);
	ulErrorStatus = read32(portAPIC_ERROR_STATUS);
	( void ) ulErrorStatus;

	/* Force an assert. */
	configASSERT( ulCriticalNesting == ~0UL );
}
/*-----------------------------------------------------------*/

#if( configUSE_COMMON_INTERRUPT_ENTRY_POINT == 1 )

	void vPortCentralInterruptHandler( uint32_t ulVector )
	{
		if( ulVector < portNUM_VECTORS )
		{
			if( xInterruptHandlerTable[ ulVector ] != NULL )
			{
				( xInterruptHandlerTable[ ulVector ] )();
			}
			else
			{
				printk("****** Unhandled exception/interrupt occurred! Vector number = 0x%x *****\n", ulVector);
			}
		} 
        else
        {
            printk("****** Unhandled exception/interrupt occurred! Vector number = 0x%x *****\n", ulVector);
        }

		/* Check for a system stack overflow. */
		configASSERT( ulSystemStack[ 10 ] == portSTACK_WORD );
		configASSERT( ulSystemStack[ 12 ] == portSTACK_WORD );
		configASSERT( ulSystemStack[ 14 ] == portSTACK_WORD );
	}

#endif /* configUSE_COMMON_INTERRUPT_ENTRY_POINT */
/*-----------------------------------------------------------*/

#if ( configUSE_COMMON_INTERRUPT_ENTRY_POINT == 1 )

	BaseType_t xPortRegisterCInterruptHandler( ISR_Handler_t pxHandler, uint32_t ulVectorNumber )
	{
	BaseType_t xReturn;

		if( prvCheckValidityOfVectorNumber( ulVectorNumber ) != pdFAIL )
		{
			/* Save the handler passed in by the application in the vector number
			passed in.  The addresses are then called from the central interrupt
			handler. */
			xInterruptHandlerTable[ ulVectorNumber ] = pxHandler;

			xReturn = pdPASS;
		}

		return xReturn;
	}

#endif /* configUSE_COMMON_INTERRUPT_ENTRY_POINT */
/*-----------------------------------------------------------*/

BaseType_t xPortInstallInterruptHandler( ISR_Handler_t pxHandler, uint32_t ulVectorNumber )
{
BaseType_t xReturn;

	if( prvCheckValidityOfVectorNumber( ulVectorNumber ) != pdFAIL )
	{
		taskENTER_CRITICAL();
		{
			/* Update the IDT to include the application defined handler. */
			prvSetInterruptGate( ( uint8_t ) ulVectorNumber, ( ISR_Handler_t ) pxHandler, portIDT_FLAGS );
		}
		taskEXIT_CRITICAL();

		xReturn = pdPASS;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvCheckValidityOfVectorNumber( uint32_t ulVectorNumber )
{
BaseType_t xReturn;

	/* Check validity of vector number. */
	if( ulVectorNumber >= portNUM_VECTORS )
	{
		/* Too high. */
		xReturn = pdFAIL;
	}
	else if( ulVectorNumber < portAPIC_MIN_ALLOWABLE_VECTOR )
	{
		/* Too low. */
		xReturn = pdFAIL;
	}
	else if( ulVectorNumber == portAPIC_TIMER_INT_VECTOR )
	{
		/* In use by FreeRTOS. */
		xReturn = pdFAIL;
	}
	else if( ulVectorNumber == portAPIC_YIELD_INT_VECTOR )
	{
		/* In use by FreeRTOS. */
		xReturn = pdFAIL;
	}
	else if( ulVectorNumber == portAPIC_LVT_ERROR_VECTOR )
	{
		/* In use by FreeRTOS. */
		xReturn = pdFAIL;
	}
	else if( ulVectorNumber == portAPIC_SPURIOUS_INT_VECTOR )
	{
		/* In use by FreeRTOS. */
		xReturn = pdFAIL;
	}
	else if( xInterruptHandlerTable[ ulVectorNumber ] != NULL )
	{
		/* Already in use by the application. */
		xReturn = pdFAIL;
	}
	else
	{
		xReturn = pdPASS;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

void vGenerateYieldInterrupt( void )
{
	__asm volatile( portYIELD_INTERRUPT );
}














