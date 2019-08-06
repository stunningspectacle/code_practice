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

#ifndef FLASH_TIMER_H
#define FLASH_TIMER_H

/*
 * Creates the LED flashing timers.  xNumberOfLEDs specifies how many timers to
 * create, with each timer toggling a different LED.  The first LED to be 
 * toggled is LED 0, with subsequent LEDs following on in numerical order.  Each
 * timer uses the exact same callback function, with the timer ID being used
 * within the callback function to determine which timer has actually expired
 * (and therefore which LED to toggle).
 */
void vStartLEDFlashTimers( UBaseType_t uxNumberOfLEDs );

#endif /* FLASH_TIMER_H */
