#include "FreeRTOS.h"
#include <portmacro.h>

void vParTestInitialise( void ){}
void vParTestSetLED( UBaseType_t uxLED, BaseType_t xValue )
{
	(void)uxLED;
	(void)xValue;
}

void vParTestToggleLED( UBaseType_t uxLED )
{
	(void)uxLED;
}
