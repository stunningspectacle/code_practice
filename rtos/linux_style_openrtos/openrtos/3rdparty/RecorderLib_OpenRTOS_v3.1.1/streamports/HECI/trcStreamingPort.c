/*******************************************************************************
 * Trace Recorder Library for Tracealyzer v3.1.0
 * Percepio AB, www.percepio.com
 *
 * trcStreamingPort.c
 *
 * Supporting functions for trace streaming, used by the "stream ports" 
 * for reading and writing data to the interface.
 * Existing ports can easily be modified to fit another setup, e.g., a 
 * different TCP/IP stack, or to define your own stream port.
 *
  * Terms of Use
 * This file is part of the trace recorder library (RECORDER), which is the 
 * intellectual property of Percepio AB (PERCEPIO) and provided under a
 * license as follows.
 * The RECORDER may be used free of charge for the purpose of recording data
 * intended for analysis in PERCEPIO products. It may not be used or modified
 * for other purposes without explicit permission from PERCEPIO.
 * You may distribute the RECORDER in its original source code form, assuming
 * this text (terms of use, disclaimer, copyright notice) is unchanged. You are
 * allowed to distribute the RECORDER with minor modifications intended for
 * configuration or porting of the RECORDER, e.g., to allow using it on a 
 * specific processor, processor family or with a specific communication
 * interface. Any such modifications should be documented directly below
 * this comment block.  
 *
 * Disclaimer
 * The RECORDER is being delivered to you AS IS and PERCEPIO makes no warranty
 * as to its use or performance. PERCEPIO does not and cannot warrant the 
 * performance or results you may obtain by using the RECORDER or documentation.
 * PERCEPIO make no warranties, express or implied, as to noninfringement of
 * third party rights, merchantability, or fitness for any particular purpose.
 * In no event will PERCEPIO, its technology partners, or distributors be liable
 * to you for any consequential, incidental or special damages, including any
 * lost profits or lost savings, even if a representative of PERCEPIO has been
 * advised of the possibility of such damages, or for any claim by any third
 * party. Some jurisdictions do not allow the exclusion or limitation of
 * incidental, consequential or special damages, or the exclusion of implied
 * warranties or limitations on how long an implied warranty may last, so the
 * above limitations may not apply to you.
 *
 * Tabs are used for indent in this file (1 tab = 4 spaces)
 *
 * Copyright Percepio AB, 2016.
 * www.percepio.com
 ******************************************************************************/

#include "trcRecorder.h"
#include <syslog.h>
#include "syslog_comps.h"

#if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)  
#if (TRC_USE_TRACEALYZER_RECORDER == 1)

/*********************************************************************************
trcHECISend the data will be copied from *data to *destdata. The real copied data 
are returned from *bytesWritten.
Note: The function is simplified: since the heci rsp buffer is 4K, and the tracealyzer
page each time is less than 3k (every 10ms). So no deal with dest buffer is smaller than source.
*********************************************************************************/
int32_t trcHECISend( void* data, int32_t size, int32_t* bytesWritten , void *destdata, int32_t destsize)
{
	int32_t datalength=size;
	if(datalength>destsize) datalength=destsize;
	/* send out tracealyzer data: copy the data to destdata.*/
	memcpy(destdata,data,datalength);
	
  	*bytesWritten=size;// ! always fake all data have been send out. SMHI will check this.
  return 0;
}

int32_t trcHECIReceive( void* data, int32_t size, int32_t* bytesRead )
{
	(void)data;
	(void)size;
	/* Read in tracealyzer command from host. It doesn't read directly from smhi heci. Instead, it return 0.*/
	*bytesRead = 0;

	return 0;
}


//////////////////////////////////////////////////////


/************** MODIFY THE ABOVE PART TO USE YOUR TPC/IP STACK ****************/

int32_t trcHECIWrite(void* data, uint32_t size, int32_t *ptrBytesWritten, void* destdata, int32_t destsize)
{
    return trcHECISend(data, size, ptrBytesWritten, destdata, destsize);
}

int32_t trcLOGWrite(void* data, uint32_t size, int32_t *ptrBytesWritten)
{
	(void )data;
 	printk("trcHECISend length = %d\n", (int)size);
  	*ptrBytesWritten=size;
  return 0;
}


int32_t trcHECIRead(void* data, uint32_t size, int32_t *ptrBytesRead)
{
     
    return trcHECIReceive(data, size, ptrBytesRead);
}

#endif /*(TRC_USE_TRACEALYZER_RECORDER == 1)*/
#endif /*(TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)*/
