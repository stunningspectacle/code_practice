/* gdt.h - IA-32 Global Descriptor Table (GDT) definitions */

/*
 * Copyright (c) 2011-2012 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
DESCRIPTION
This file provides definitions for the Global Descriptor Table (GDT) for the
IA-32 architecture.
*/

#ifndef _GDT_H
#define _GDT_H

/* includes */

#ifndef _ASMLANGUAGE

#include <stdint.h>

/* typedefs */

/* a generic GDT entry structure definition */

typedef struct s_gdtDesc
    {
        uint32_t low;
        uint32_t high;
    } tGdtDesc;

/*
 * structure definition for the GDT "header"
 * (does not include the GDT entries).
 * The structure is packed to force the structure to appear "as is".
 * Unfortunately, this appears to remove any alignment restrictions
 * so the aligned attribute is used.
 */

typedef struct __attribute__((__packed__)) s_gdtHeader
    {
    uint16_t	limit;		/* GDT limit */
    tGdtDesc *	pEntries;	/* pointer to the GDT entries */
    } tGdtHeader __attribute__((aligned(4)));
 
/* externs */

extern tGdtHeader _Gdt;      /* GDT is implemented in Intel/core/gdt.c */
    
extern unsigned int nanoCpuGdtAdd(
        uint32_t  desc1,     /* 1st half of raw descriptor data */
        uint32_t  desc2      /* 2nd half of raw descriptor data */
 );

#endif /* _ASMLANGUAGE */
#endif /* _GDT_H */
