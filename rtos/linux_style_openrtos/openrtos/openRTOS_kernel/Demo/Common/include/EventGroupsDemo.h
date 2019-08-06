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
 * This file contains fairly comprehensive checks on the behaviour of event
 * groups.  It is not intended to be a user friendly demonstration of the event
 * groups API.
 */

#ifndef EVENT_GROUPS_DEMO_H
#define EVENT_GROUPS_DEMO_H

void vStartEventGroupTasks( void );
BaseType_t xAreEventGroupTasksStillRunning( void );
void vPeriodicEventGroupsProcessing( void );

#endif /* EVENT_GROUPS_DEMO_H */

