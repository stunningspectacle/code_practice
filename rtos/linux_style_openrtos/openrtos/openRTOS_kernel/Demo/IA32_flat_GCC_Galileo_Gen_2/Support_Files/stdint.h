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
#ifndef FREERTOS_STDINT
#define FREERTOS_STDINT

/*******************************************************************************
 * THIS IS NOT A FULL stdint.h IMPLEMENTATION - It only contains the definitions
 * necessary to build the FreeRTOS code.  It is provided to allow FreeRTOS to be
 * built using compilers that do not provide their own stdint.h definition.
 *
 * To use this file:
 *
 *    1) Copy this file into the directory that contains your FreeRTOSConfig.h
 *       header file, as that directory will already be in the compilers include
 *       path.
 *
 *    2) Rename the copied file stdint.h.
 *
 */

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef uint32_t uintn_t;

#endif /* FREERTOS_STDINT */
