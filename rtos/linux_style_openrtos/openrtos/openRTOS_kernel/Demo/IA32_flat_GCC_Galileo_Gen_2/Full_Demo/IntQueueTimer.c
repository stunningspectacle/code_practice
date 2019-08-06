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

/*
 * Provides the port specific part of the standard IntQ test, which is
 * implemented in FreeRTOS/Demo/Common/Minimal/IntQueue.c.  Three HPET timers
 * are used to generate the interrupts.  The timers are configured in
 * prvSetupHardware(), in main.c.
 */


/* Scheduler includes. */
#include "FreeRTOS.h"

/* Demo includes. */
#include "IntQueueTimer.h"
#include "IntQueue.h"

/*
 * Prototypes of the callback functions which are called from the HPET timer
 * support file.  For demonstration purposes, timer 0 and timer 1 are standard
 * C functions that use the central interrupt handler, and are installed using
 * xPortRegisterCInterruptHandler() - and timer 2 uses its own interrupt entry
 * asm wrapper code and is installed using xPortInstallInterruptHandler().  For
 * convenience the asm wrapper which calls vApplicationHPETTimer1Handler(), is
 * implemented in RegTest.S.  See
 * http://www.freertos.org/RTOS_Intel_Quark_Galileo_GCC.html#interrupts for more
 * details.
 */
void vApplicationHPETTimer0Handler( void );
void vApplicationHPETTimer1Handler( void );
void vApplicationHPETTimer2Handler( void );

/*
 * Set to pdTRUE when vInitialiseTimerForIntQueueTest() is called so the timer
 * callback functions know the scheduler is running and the tests can run.
 */
static volatile BaseType_t xSchedulerRunning = pdFALSE;

/* Used to count the nesting depth to ensure the test is testing what it is
intended to test. */
static volatile uint32_t ulMaxInterruptNesting = 0;
extern volatile uint32_t ulInterruptNesting;

/*-----------------------------------------------------------*/

void vInitialiseTimerForIntQueueTest( void )
{
	/* The HPET timers are set up in main(), before the scheduler is started,
	so there is nothing to do here other than note the scheduler is now running.
	This could be done by calling a FreeRTOS API function, but its convenient
	and efficient just to store the fact in a file scope variable. */
	xSchedulerRunning = pdTRUE;
}
/*-----------------------------------------------------------*/

void vApplicationHPETTimer0Handler( void )
{
BaseType_t xHigherPriorityTaskWoken;

	if( xSchedulerRunning != pdFALSE )
	{
		if( ulInterruptNesting > ulMaxInterruptNesting )
		{
			ulMaxInterruptNesting = ulInterruptNesting;
		}

		xHigherPriorityTaskWoken = xFirstTimerHandler();
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
/*-----------------------------------------------------------*/

void vApplicationHPETTimer1Handler( void )
{
BaseType_t xHigherPriorityTaskWoken;

	if( xSchedulerRunning != pdFALSE )
	{
		if( ulInterruptNesting > ulMaxInterruptNesting )
		{
			ulMaxInterruptNesting = ulInterruptNesting;
		}

		xHigherPriorityTaskWoken = xSecondTimerHandler();
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
/*-----------------------------------------------------------*/

void vApplicationHPETTimer2Handler( void )
{
	if( ulInterruptNesting > ulMaxInterruptNesting )
	{
		ulMaxInterruptNesting = ulInterruptNesting;
	}
}

