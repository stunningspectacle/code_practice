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

/**
 * This version of flash .c is for use on systems that have limited stack space
 * and no display facilities.  The complete version can be found in the 
 * Demo/Common/Full directory.
 * 
 * Three tasks are created, each of which flash an LED at a different rate.  The first 
 * LED flashes every 200ms, the second every 400ms, the third every 600ms.
 *
 * The LED flash tasks provide instant visual feedback.  They show that the scheduler 
 * is still operational.
 *
 */


#include <stdlib.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo program include files. */
#include "partest.h"
#include "flash.h"

#define ledSTACK_SIZE		configMINIMAL_STACK_SIZE
#define ledNUMBER_OF_LEDS	( 3 )
#define ledFLASH_RATE_BASE	( ( TickType_t ) 333 )

/* Variable used by the created tasks to calculate the LED number to use, and
the rate at which they should flash the LED. */
static volatile UBaseType_t uxFlashTaskNumber = 0;

/* The task that is created three times. */
static portTASK_FUNCTION_PROTO( vLEDFlashTask, pvParameters );

/*-----------------------------------------------------------*/

void vStartLEDFlashTasks( UBaseType_t uxPriority )
{
BaseType_t xLEDTask;

	/* Create the three tasks. */
	for( xLEDTask = 0; xLEDTask < ledNUMBER_OF_LEDS; ++xLEDTask )
	{
		/* Spawn the task. */
		xTaskCreate( vLEDFlashTask, "LEDx", ledSTACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
	}
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vLEDFlashTask, pvParameters )
{
TickType_t xFlashRate, xLastFlashTime;
UBaseType_t uxLED;

	/* The parameters are not used. */
	( void ) pvParameters;

	/* Calculate the LED and flash rate. */
	portENTER_CRITICAL();
	{
		/* See which of the eight LED's we should use. */
		uxLED = uxFlashTaskNumber;

		/* Update so the next task uses the next LED. */
		uxFlashTaskNumber++;
	}
	portEXIT_CRITICAL();

	xFlashRate = ledFLASH_RATE_BASE + ( ledFLASH_RATE_BASE * ( TickType_t ) uxLED );
	xFlashRate /= portTICK_PERIOD_MS;

	/* We will turn the LED on and off again in the delay period, so each
	delay is only half the total period. */
	xFlashRate /= ( TickType_t ) 2;

	/* We need to initialise xLastFlashTime prior to the first call to 
	vTaskDelayUntil(). */
	xLastFlashTime = xTaskGetTickCount();

	for(;;)
	{
		/* Delay for half the flash period then turn the LED on. */
		vTaskDelayUntil( &xLastFlashTime, xFlashRate );
		vParTestToggleLED( uxLED );

		/* Delay for half the flash period then turn the LED off. */
		vTaskDelayUntil( &xLastFlashTime, xFlashRate );
		vParTestToggleLED( uxLED );
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

