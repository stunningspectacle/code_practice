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
 * This is a version of PollQ.c that uses the alternative (Alt) API.
 * 
 * Creates two tasks that communicate over a single queue.  One task acts as a
 * producer, the other a consumer.
 *
 * The producer loops for three iteration, posting an incrementing number onto the
 * queue each cycle.  It then delays for a fixed period before doing exactly the
 * same again.
 *
 * The consumer loops emptying the queue.  Each item removed from the queue is
 * checked to ensure it contains the expected value.  When the queue is empty it
 * blocks for a fixed period, then does the same again.
 *
 * All queue access is performed without blocking.  The consumer completely empties
 * the queue each time it runs so the producer should never find the queue full.
 *
 * An error is flagged if the consumer obtains an unexpected value or the producer
 * find the queue is full.
 */

/*
Changes from V2.0.0

	+ Delay periods are now specified using variables and constants of
	  TickType_t rather than uint32_t.
*/

#include <stdlib.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo program include files. */
#include "AltPollQ.h"

#define pollqSTACK_SIZE			configMINIMAL_STACK_SIZE
#define pollqQUEUE_SIZE			( 10 )
#define pollqPRODUCER_DELAY		( ( TickType_t ) 200 / portTICK_PERIOD_MS )
#define pollqCONSUMER_DELAY		( pollqPRODUCER_DELAY - ( TickType_t ) ( 20 / portTICK_PERIOD_MS ) )
#define pollqNO_DELAY			( ( TickType_t ) 0 )
#define pollqVALUES_TO_PRODUCE	( ( BaseType_t ) 3 )
#define pollqINITIAL_VALUE		( ( BaseType_t ) 0 )

/* The task that posts the incrementing number onto the queue. */
static portTASK_FUNCTION_PROTO( vPolledQueueProducer, pvParameters );

/* The task that empties the queue. */
static portTASK_FUNCTION_PROTO( vPolledQueueConsumer, pvParameters );

/* Variables that are used to check that the tasks are still running with no
errors. */
static volatile BaseType_t xPollingConsumerCount = pollqINITIAL_VALUE, xPollingProducerCount = pollqINITIAL_VALUE;

/*-----------------------------------------------------------*/

void vStartAltPolledQueueTasks( UBaseType_t uxPriority )
{
static QueueHandle_t xPolledQueue;

	/* Create the queue used by the producer and consumer. */
	xPolledQueue = xQueueCreate( pollqQUEUE_SIZE, ( UBaseType_t ) sizeof( uint16_t ) );

	/* vQueueAddToRegistry() adds the queue to the queue registry, if one is
	in use.  The queue registry is provided as a means for kernel aware 
	debuggers to locate queues and has no purpose if a kernel aware debugger
	is not being used.  The call to vQueueAddToRegistry() will be removed
	by the pre-processor if configQUEUE_REGISTRY_SIZE is not defined or is 
	defined to be less than 1. */
	vQueueAddToRegistry( xPolledQueue, "AltPollQueue" );


	/* Spawn the producer and consumer. */
	xTaskCreate( vPolledQueueConsumer, "QConsNB", pollqSTACK_SIZE, ( void * ) &xPolledQueue, uxPriority, ( TaskHandle_t * ) NULL );
	xTaskCreate( vPolledQueueProducer, "QProdNB", pollqSTACK_SIZE, ( void * ) &xPolledQueue, uxPriority, ( TaskHandle_t * ) NULL );
}
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vPolledQueueProducer, pvParameters )
{
uint16_t usValue = ( uint16_t ) 0;
BaseType_t xError = pdFALSE, xLoop;

	#ifdef USE_STDIO
	void vPrintDisplayMessage( const char * const * ppcMessageToSend );
	
		const char * const pcTaskStartMsg = "Alt polling queue producer task started.\r\n";

		/* Queue a message for printing to say the task has started. */
		vPrintDisplayMessage( &pcTaskStartMsg );
	#endif

	for( ;; )
	{		
		for( xLoop = 0; xLoop < pollqVALUES_TO_PRODUCE; xLoop++ )
		{
			/* Send an incrementing number on the queue without blocking. */
			if( xQueueAltSendToBack( *( ( QueueHandle_t * ) pvParameters ), ( void * ) &usValue, pollqNO_DELAY ) != pdPASS )
			{
				/* We should never find the queue full so if we get here there
				has been an error. */
				xError = pdTRUE;
			}
			else
			{
				if( xError == pdFALSE )
				{
					/* If an error has ever been recorded we stop incrementing the
					check variable. */
					portENTER_CRITICAL();
						xPollingProducerCount++;
					portEXIT_CRITICAL();
				}

				/* Update the value we are going to post next time around. */
				usValue++;
			}
		}

		/* Wait before we start posting again to ensure the consumer runs and
		empties the queue. */
		vTaskDelay( pollqPRODUCER_DELAY );
	}
}  /*lint !e818 Function prototype must conform to API. */
/*-----------------------------------------------------------*/

static portTASK_FUNCTION( vPolledQueueConsumer, pvParameters )
{
uint16_t usData, usExpectedValue = ( uint16_t ) 0;
BaseType_t xError = pdFALSE;

	#ifdef USE_STDIO
	void vPrintDisplayMessage( const char * const * ppcMessageToSend );
	
		const char * const pcTaskStartMsg = "Alt blocking queue consumer task started.\r\n";

		/* Queue a message for printing to say the task has started. */
		vPrintDisplayMessage( &pcTaskStartMsg );
	#endif

	for( ;; )
	{		
		/* Loop until the queue is empty. */
		while( uxQueueMessagesWaiting( *( ( QueueHandle_t * ) pvParameters ) ) )
		{
			if( xQueueAltReceive( *( ( QueueHandle_t * ) pvParameters ), &usData, pollqNO_DELAY ) == pdPASS )
			{
				if( usData != usExpectedValue )
				{
					/* This is not what we expected to receive so an error has
					occurred. */
					xError = pdTRUE;

					/* Catch-up to the value we received so our next expected
					value should again be correct. */
					usExpectedValue = usData;
				}
				else
				{
					if( xError == pdFALSE )
					{
						/* Only increment the check variable if no errors have
						occurred. */
						portENTER_CRITICAL();
							xPollingConsumerCount++;
						portEXIT_CRITICAL();
					}
				}

				/* Next time round we would expect the number to be one higher. */
				usExpectedValue++;
			}
		}

		/* Now the queue is empty we block, allowing the producer to place more
		items in the queue. */
		vTaskDelay( pollqCONSUMER_DELAY );
	}
} /*lint !e818 Function prototype must conform to API. */
/*-----------------------------------------------------------*/

/* This is called to check that all the created tasks are still running with no errors. */
BaseType_t xAreAltPollingQueuesStillRunning( void )
{
BaseType_t xReturn;

	/* Check both the consumer and producer poll count to check they have both
	been changed since out last trip round.  We do not need a critical section
	around the check variables as this is called from a higher priority than
	the other tasks that access the same variables. */
	if( ( xPollingConsumerCount == pollqINITIAL_VALUE ) ||
		( xPollingProducerCount == pollqINITIAL_VALUE )
	  )
	{
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	/* Set the check variables back down so we know if they have been
	incremented the next time around. */
	xPollingConsumerCount = pollqINITIAL_VALUE;
	xPollingProducerCount = pollqINITIAL_VALUE;

	return xReturn;
}
