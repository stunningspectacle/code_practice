/*
 * main.c
 *
 *  Created on: May 3, 2015
 *      Author: abrill
 */

/*
 * INTEL CONFIDENTIAL

 * Copyright 2012-2015 Intel Corporation All Rights Reserved.

 * The source code contained or described herein and all documents related to the source
 * code ("Material") are owned by Intel Corporation or its suppliers or licensors. Title to
 * the Material remains with Intel Corporation or its suppliers and licensors. The Material
 * contains trade secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors. The Material is protected by worldwide copyright and trade
 * secret laws and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted, distributed, or disclosed
 * in any way without Intel's prior express written permission.

 *

 *    No license under any patent, copyright, trade secret or other intellectual property
 * right is granted to or conferred upon you by disclosure or delivery of the Materials,
 * either expressly, by implication, inducement, estoppel or otherwise. Any license under
 * such intellectual property rights must be express and approved by Intel in writing.
 *   Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
 * or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way
 */

/******************************************************************************
 * NOTE 1:  This project provides two demo applications.  A simple blinky
 * style project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLY_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLY_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the comprehensive version.
 *
 * NOTE 2:  This file only contains the source code that is specific to the
 * full demo.  Generic functions, such FreeRTOS hook functions, and functions
 * required to configure the hardware, are defined in main.c.
 *
 ******************************************************************************
 *
 * main_full() creates all the demo application tasks and software timers, then
 * starts the scheduler.  The web documentation provides more details of the
 * standard demo application tasks, which provide no particular functionality,
 * but do provide a good example of how to use the FreeRTOS API.
 *
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Reg test" tasks - These fill both the core and floating point registers with
 * known values, then check that each register maintains its expected value for
 * the lifetime of the task.  Each task uses a different set of values.  The reg
 * test tasks execute with a very low priority, so get preempted very
 * frequently.  A register containing an unexpected value is indicative of an
 * error in the context switching mechanism.
 *
 * "Check" task - The check task period is initially set to three seconds.  The
 * task checks that all the standard demo tasks, and the register check tasks,
 * are not only still executing, but are executing without reporting any errors.
 * If the check task discovers that a task has either stalled, or reported an
 * error, then it changes its own execution period from the initial three
 * seconds, to just 200ms.  The check task also toggles an LED each time it is
 * called.  This provides a visual indication of the system status:  If the LED
 * toggles every three seconds, then no issues have been discovered.  If the LED
 * toggles every 200ms, then an issue has been discovered with at least one
 * task.
 */


#include "FreeRTOS.h"
/* Scheduler includes. */
#include "task.h"

/* Standard demo application includes. */
#include "flop.h"
#include "semtest.h"
#include "dynamic.h"
#include "BlockQ.h"
#include "blocktim.h"
#include "countsem.h"
#include "GenQTest.h"
#include "recmutex.h"
#include "death.h"
#include "partest.h"
#include "comtest2.h"
#include "serial.h"
#include "TimerDemo.h"
#include "QueueOverwrite.h"
#include "EventGroupsDemo.h"
#include "IntSemTest.h"
#include "PollQ.h"
#include "integer.h"
#include "exception_support.h"
#include <defs.h>

#include <ia32_interrupts.h>

/* Priorities for the demo application tasks. */
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2UL )
#define mainCREATOR_TASK_PRIORITY			( tskIDLE_PRIORITY + 3UL )
#define mainFLOP_TASK_PRIORITY				( tskIDLE_PRIORITY )
#define mainUART_COMMAND_CONSOLE_STACK_SIZE	( configMINIMAL_STACK_SIZE * 3UL )
#define mainCOM_TEST_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( configMAX_PRIORITIES - 1 )
#define mainQUEUE_OVERWRITE_PRIORITY		( tskIDLE_PRIORITY )

/* The priority used by the UART command console task.  This is very basic and
uses the Altera polling UART driver - so *must* run at the idle priority. */
#define mainUART_COMMAND_CONSOLE_TASK_PRIORITY	( tskIDLE_PRIORITY )

/* The LED used by the check timer. */
#define mainCHECK_LED						( 0 )

/* A block time of zero simply means "don't block". */
#define mainDONT_BLOCK						( 0UL )

/* The period after which the check timer will expire, in ms, provided no errors
have been reported by any of the standard demo tasks.  ms are converted to the
equivalent in ticks using the portTICK_PERIOD_MS constant. */
#define mainNO_ERROR_CHECK_TASK_PERIOD		( 10000UL / portTICK_PERIOD_MS )

/* The period at which the check timer will expire, in ms, if an error has been
reported in one of the standard demo tasks.  ms are converted to the equivalent
in ticks using the portTICK_PERIOD_MS constant. */
//#define mainERROR_CHECK_TASK_PERIOD 		( 200UL / portTICK_PERIOD_MS )
#define mainERROR_CHECK_TASK_PERIOD 		( 4000UL / portTICK_PERIOD_MS )

/* Parameters that are passed into the register check tasks solely for the
purpose of ensuring parameters are passed into tasks correctly. */
#define mainREG_TEST_TASK_1_PARAMETER		( ( void * ) 0x12345678 )
#define mainREG_TEST_TASK_2_PARAMETER		( ( void * ) 0x87654321 )

/* The base period used by the timer test tasks. */
#define mainTIMER_TEST_PERIOD				( 50 )


/* The following two variables are used to communicate the status of the
register check tasks to the check task.  If the variables keep incrementing,
then the register check tasks has not discovered any errors.  If a variable
stops incrementing, then an error has been found. */
volatile unsigned long ulRegTest1LoopCounter = 0UL, ulRegTest2LoopCounter = 0UL;


void i2clib_start(void);
void i2c_out(char* str);

void syslog(int priority, const char *message, ...);


__pinned_kernel_code__ void vApplicationTickHook( void )
{
//	/* The full demo includes a software timer demo/test that requires
//	prodding periodically from the tick interrupt. */
	vTimerPeriodicISRTests();

	/* Call the periodic queue overwrite from ISR demo. */
	vQueueOverwritePeriodicISRDemo();

	/* Call the periodic event group from ISR demo. */
	vPeriodicEventGroupsProcessing();

	/* Call the periodic test that uses mutexes form an interrupt. */
	vInterruptSemaphorePeriodicTest();
}


static void prvRegTestTaskEntry1( void *pvParameters )
{
	/* Although the regtest task is written in assembler, its entry point is
	written in C for convenience of checking the task parameter is being passed
	in correctly. */
	if( pvParameters == mainREG_TEST_TASK_1_PARAMETER )
	{
		/* The reg test task also tests the floating point registers.  Tasks
		that use the floating point unit must call vPortTaskUsesFPU() before
		any floating point instructions are executed. */
		vPortTaskUsesFPU();

		/* Start the part of the test that is written in assembler. */
		//vRegTest1Implementation();
	}

	/* The following line will only execute if the task parameter is found to
	be incorrect.  The check timer will detect that the regtest loop counter is
	not being incremented and flag an error. */
	vTaskDelete( NULL );
}
//*-----------------------------------------------------------*/
//
static void prvRegTestTaskEntry2( void *pvParameters )
{
	/* Although the regtest task is written in assembler, its entry point is
	written in C for convenience of checking the task parameter is being passed
	in correctly. */
	if( pvParameters == mainREG_TEST_TASK_2_PARAMETER )
	{
		/* The reg test task also tests the floating point registers.  Tasks
		that use the floating point unit must call vPortTaskUsesFPU() before
		any floating point instructions are executed. */
		vPortTaskUsesFPU();

		/* Start the part of the test that is written in assembler. */
		//vRegTest2Implementation();
	}

	/* The following line will only execute if the task parameter is found to
	be incorrect.  The check timer will detect that the regtest loop counter is
	not being incremented and flag an error. */
	vTaskDelete( NULL );
}
///*-----------------------------------------------------------*/
//
//static void prvPseudoRandomiser( void *pvParameters )
//{
//const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL, ulMinDelay = ( 35 / portTICK_PERIOD_MS );
//volatile uint32_t ulNextRand = ( uint32_t ) &pvParameters, ulValue;
//
//
//	/* This task does nothing other than ensure there is a little bit of
//	disruption in the scheduling pattern of the other tasks.  Normally this is
//	done by generating interrupts at pseudo random times. */
//	for( ;; )
//	{
//		ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
//		ulValue = ( ulNextRand >> 16UL ) & 0xffUL;
//
//		if( ulValue < ulMinDelay )
//		{
//			ulValue = ulMinDelay;
//		}
//
//		vTaskDelay( ulValue );
//
//		while( ulValue > 0 )
//		{
//			__asm volatile( "NOP" );
//			__asm volatile( "NOP" );
//			__asm volatile( "NOP" );
//			__asm volatile( "NOP" );
//
//			ulValue--;
//		}
//	}
//}


void vPortSysTickHandler(void *context, unsigned id);



//

void vApplicationIdleHook( void )
{


}

static void prvCheckTask( void *pvParameters )
{
TickType_t xDelayPeriod = mainNO_ERROR_CHECK_TASK_PERIOD;
TickType_t xLastExecutionTime;
//static unsigned long ulLastRegTest1Value = 0, ulLastRegTest2Value = 0;
unsigned long ulErrorFound = pdFALSE;

	/* Just to stop compiler warnings. */
	( void ) pvParameters;

	/* Initialise xLastExecutionTime so the first call to vTaskDelayUntil()
	works correctly. */
	xLastExecutionTime = xTaskGetTickCount();

	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error.  The onboard LED is toggled on each iteration.
	If an error is detected then the delay period is decreased from
	mainNO_ERROR_CHECK_TASK_PERIOD to mainERROR_CHECK_TASK_PERIOD.  This has the
	effect of increasing the rate at which the onboard LED toggles, and in so
	doing gives visual feedback of the system status. */
	for( ;; )
	{
		ulErrorFound = 0;
		/* Delay until it is time to execute again. */
		vTaskDelayUntil( &xLastExecutionTime, xDelayPeriod );

		/* Check all the demo tasks (other than the flash tasks) to ensure
		that they are all still running, and that none have detected an error. */
		if( xAreMathsTaskStillRunning() != pdTRUE ) //ok
		{
			ulErrorFound = 1 << 1;
		}

		if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 2);
		}

		if( xAreBlockingQueuesStillRunning() != pdTRUE ) //ok
		{
			ulErrorFound |= (1 << 3);
		}

		if ( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 4);
		}

		if ( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 5);
		}

		if ( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 6);
		}

//							if( xIsCreateTaskStillRunning() != pdTRUE )
//							{
//								ulErrorFound = 1 << 7;
//							}

		if( xAreSemaphoreTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 8);
		}

		if( xAreTimerDemoTasksStillRunning( ( TickType_t ) mainNO_ERROR_CHECK_TASK_PERIOD ) != pdPASS )
		{
			ulErrorFound |= (1 << 9);
		}

		if( xAreCountingSemaphoreTasksStillRunning() != pdTRUE )//X
		{
			ulErrorFound |= (1 << 10);
		}

		if( xIsQueueOverwriteTaskStillRunning() != pdPASS )
		{
			ulErrorFound |= (1 << 11);
		}

		if( xAreEventGroupTasksStillRunning() != pdPASS )
		{
			ulErrorFound |= (1 << 12);
		}

		if( xAreInterruptSemaphoreTasksStillRunning() != pdPASS )
		{
			ulErrorFound |= (1 << 13);
		}

		/*if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 14);
		}

		if( xArePollingQueuesStillRunning() != pdTRUE )
		{
			ulErrorFound |= (1 << 15);
		}*/
//								/* Check that the register test 1 task is still running. */
//								if( ulLastRegTest1Value == ulRegTest1LoopCounter )
//								{
//									ulErrorFound = 1 << 14;
//								}
//								ulLastRegTest1Value = ulRegTest1LoopCounter;
//
//								/* Check that the register test 2 task is still running. */
//								if( ulLastRegTest2Value == ulRegTest2LoopCounter )
//								{
//									ulErrorFound = 1 << 15;
//								}
//								ulLastRegTest2Value = ulRegTest2LoopCounter;

		/* Toggle the check LED to give an indication of the system status.  If
		the LED toggles every mainNO_ERROR_CHECK_TASK_PERIOD milliseconds then
		everything is ok.  A faster toggle indicates an error. */
		vParTestToggleLED( mainCHECK_LED );

		if( ulErrorFound != pdFALSE )
		{
			/* An error has been detected in one of the tasks - flash the LED
			at a higher frequency to give visible feedback that something has
			gone wrong (it might just be that the loop back connector required
			by the comtest tasks has not been fitted). */
			xDelayPeriod = mainERROR_CHECK_TASK_PERIOD;
			syslog(0, "TESTS FAILED....\n");
		}
		else{
			syslog(0, "ALL TESTS PASSED!\n");
		}
	}
}

typedef double  double_t;

inline extern
double_t            fabs(     double_t     x)
{
    return (x >= 0) ? x : -x;
}

void preSchedulerCode(void)
{
	vStartTimerDemoTask( mainTIMER_TEST_PERIOD ); //ok
	vStartQueueOverwriteTask( mainQUEUE_OVERWRITE_PRIORITY ); // X

}

void tasks(void)
{
	/* Start all the other standard demo/test tasks.  They have not particular
	functionality, but do demonstrate how to use the FreeRTOS API and test the
	kernel port. */
	extern void trace_worker();

	//vStartIntegerMathTasks( tskIDLE_PRIORITY );
	//vStartPolledQueueTasks( tskIDLE_PRIORITY );
	xTaskCreate(trace_worker, "trace_worker", 1024, NULL, 3, NULL );

	vCreateBlockTimeTasks();
	vStartDynamicPriorityTasks(); // X
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY ); //ok
	vStartCountingSemaphoreTasks(); // X
	vStartGenericQueueTasks( tskIDLE_PRIORITY ); // OK
	vStartRecursiveMutexTasks(); // X
	vStartSemaphoreTasks( mainSEM_TEST_PRIORITY ); // X
	vStartMathTasks( mainFLOP_TASK_PRIORITY ); //ok
	vStartEventGroupTasks(); // X
	vStartInterruptSemaphoreTasks(); // X

//
//	/* Create the register check tasks, as described at the top of this	file */
	xTaskCreate( prvRegTestTaskEntry1, "Reg1", configMINIMAL_STACK_SIZE, mainREG_TEST_TASK_1_PARAMETER, tskIDLE_PRIORITY, NULL );
	xTaskCreate( prvRegTestTaskEntry2, "Reg2", configMINIMAL_STACK_SIZE, mainREG_TEST_TASK_2_PARAMETER, tskIDLE_PRIORITY, NULL );

	/* Create the task that just adds a little random behaviour. */
//	xTaskCreate( prvPseudoRandomiser, "Rnd", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL );

	/* Create the task that performs the 'check' functionality,	as described at
	the top of this file. */
	xTaskCreate( prvCheckTask, "Check", 1024, NULL, mainCHECK_TASK_PRIORITY, NULL );

	/* The set of tasks created by the following function call have to be
	created last as they keep account of the number of tasks they expect to see
	running. */
//	vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

}

int main(void* param)
{
	(void) param;
	syslog(0, "OpenRTOS demo started\n");
	while (1) {
		vTaskSuspend(xTaskGetCurrentTaskHandle());
	}
}



