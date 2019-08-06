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

#ifndef CRFLASH_LED_H
#define CRFLASH_LED_H

/*
 * Create the co-routines used to flash the LED's at different rates.
 *
 * @param uxPriority The number of 'fixed delay' co-routines to create.  This
 *		  also effects the number of LED's that will be utilised.  For example,
 *		  passing in 3 will cause LED's 0 to 2 to be utilised.
 */
void vStartFlashCoRoutines( UBaseType_t uxPriority );

/*
 * Return pdPASS or pdFAIL depending on whether an error has been detected
 * or not.
 */
BaseType_t xAreFlashCoRoutinesStillRunning( void );

#endif

