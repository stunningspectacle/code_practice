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
 * The library functions not provided by libgcc for freestanding environments.
 * The implementation of the functions in this file have made NO attempt
 * whatsoever to be optimised!
 */

// INTEL CHANGE: all warnings being treated as errors, so suppress this warning here to avoid compilation errors. 
// #warning The functions in this file are very basic, and not optimised.

#include <stddef.h>

/*-----------------------------------------------------------*/

void *memcpy( void *pvDest, const void *pvSource, size_t xBytes )
{
/* The compiler used during development seems to err unless these volatiles are
included at -O3 optimisation.  */
volatile unsigned char *pcDest = ( volatile unsigned char * ) pvDest, *pcSource = ( volatile unsigned char * ) pvSource;
size_t x;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */
	if( pvDest != pvSource )
	{
		for( x = 0; x < xBytes; x++ )
		{
			pcDest[ x ] = pcSource[ x ];
		}
	}

	return pvDest;
}
/*-----------------------------------------------------------*/

void *memset( void *pvDest, int iValue, size_t xBytes )
{
/* The compiler used during development seems to err unless these volatiles are
included at -O3 optimisation.  */
volatile unsigned char * volatile pcDest = ( volatile unsigned char * volatile ) pvDest;
volatile size_t x;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */
	for( x = 0; x < xBytes; x++ )
	{
		pcDest[ x ] = ( unsigned char ) iValue;
	}

	return pvDest;
}
/*-----------------------------------------------------------*/

int memcmp( const void *pvMem1, const void *pvMem2, unsigned long ulBytes )
{
const volatile unsigned char *pucMem1 = pvMem1, *pucMem2 = pvMem2;
register unsigned long x;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */
    for( x = 0; x < ulBytes; x++ )
    {
        if( pucMem1[ x ] != pucMem2[ x ] )
        {
            break;
        }
    }

    return ulBytes - x;
}
/*-----------------------------------------------------------*/

int strcmp( const char *pcString1, const char *pcString2 )
{
volatile int iReturn, iIndex = 0;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */

	while( ( pcString1[ iIndex ] != 0x00 ) && ( pcString2[ iIndex ] != 0x00 ) )
	{
		iIndex++;
	}

	if( ( pcString1[ iIndex ] == 0x00 ) && ( pcString2[ iIndex ] == 0x00 ) )
	{
		iReturn = 0;
	}
	else
	{
		iReturn = ~0;
	}

	return iReturn;
}


