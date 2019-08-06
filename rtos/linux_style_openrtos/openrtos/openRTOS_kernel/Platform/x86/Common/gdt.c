/* gdt.c - Global Descriptor Table support */

/*
 * Copyright (c) 2011-2014 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
DESCRIPTION
This module contains routines for updating the global descriptor table (GDT)
for the IA-32 architecture.
*/

/* includes */

#include <mmu.h>
#include <mmio.h>

#include <proc.h>
#include <ish_sections.h>
#include <gdt.h>
#include <aon_shared.h>

/* defines */

/* 0:15 - limit(low). 16:31 - base(low) */
#define CREATE_GDT_SELECTOR_LOW(base, limit) ((((limit) >> 12) & 0xFFFF) | (((base) & 0xFFFF) << 16 ))

/* 0:7 - base (mid). 8-15 - flags. 16-23 - limit(high). 24-31 - base(high). */
#define CREATE_GDT_SELECTOR_HIGH(base, limit, flags) ((((base) >> 16) & 0xFF) | (((flags) << 8)  & 0xFF00) | (((limit) >> 12) & 0xFF0000) | (((base))  & 0xFF000000) | 0xC00000)

#define NUM_BASE_GDT_ENTRIES 9
#define CONFIG_NUM_GDT_SPARE_ENTRIES 3

#define MAX_GDT_ENTRIES	(NUM_BASE_GDT_ENTRIES + CONFIG_NUM_GDT_SPARE_ENTRIES)

extern unsigned long __cs_selector_low;
extern unsigned long __cs_selector_high;
extern unsigned long __ds_selector_low;
extern unsigned long __ds_selector_high;
extern unsigned long __ss_selector_low;
extern unsigned long __ss_selector_high;
extern unsigned long __tss_selector_low;
extern unsigned long __tss_selector_high;

extern unsigned long mmuPageDirectoryPhys;

#define cs_selector_low   ((unsigned long)&__cs_selector_low)
#define cs_selector_high   ((unsigned long)&__cs_selector_high)
#define ds_selector_low   ((unsigned long)&__ds_selector_low)
#define ds_selector_high   ((unsigned long)&__ds_selector_high)
#define ss_selector_low   ((unsigned long)&__ss_selector_low)
#define ss_selector_high   ((unsigned long)&__ss_selector_high)
#define tss_selector_low   ((unsigned long)&__tss_selector_low)
#define tss_selector_high   ((unsigned long)&__tss_selector_high)

#define mmuPDtPhys       ((unsigned long)&mmuPageDirectoryPhys)

/* locals */

/* for aon task switch */
__stationary_kernel_data__ tss_t _TssDes =
{
    .prevTaskLink     = 0,
    .reserved1        = 0,
    .esp0             = 0,
    .ss0              = 0,
    .reserved2        = 0,
    .esp1             = 0,
    .ss1              = 0,
    .reserved3        = 0,
    .esp2             = 0,
    .ss2              = 0,
    .reserved4        = 0,
    .cr3              = mmuPDtPhys,     
    .eip              = 0,
    .eflags           = 0,
    .eax              = 0,
    .ecx              = 0,
    .edx              = 0,
    .ebx              = 0,
    .esp              = 0, 
    .ebp              = 0,
    .esi              = 0,
    .edi              = 0,                  
    .es               = 0,
    .reserved5        = 0,
    .cs               = 0,
    .reserved6        = 0,
    .ss               = 0,
    .reserved7        = 0,
    .ds               = 0,
    .reserved8        = 0,
    .fs               = 0,
    .reserved9        = 0,
    .gs               = 0,
    .reserved10       = 0,                  
    .ldt_seg_selector = 0,
    .reserved11       = 0,
    .trap_debug       = 0,
    .iomap_base_adrs  = 0x67,		/* ioMapBaseAddrs */
};

/*
 * The RAM based global descriptor table. It is aligned on an 8 byte boundary
 * as the Intel manuals recommend this for best performance.
 */
__stationary_kernel_data__ tGdtDesc _GdtEntries[MAX_GDT_ENTRIES] __attribute__((aligned(8))) =
{
    /* Entry 0: NULL descriptor */

    {        
        0x0,
        0x0,
    },

	/* Entry 1 : CS (after paging enabled), segment selector = 0x0008. */
    {
        cs_selector_low,
        cs_selector_high,
    },

	/* Entry 2 : DS (after paging enabled), segment selector = 0x0010. */
    {
        ds_selector_low,
        ds_selector_high,
    },

	/* Entry 3: CS: Logical-to-phys while MMU is off, segment selector = 0x0018.  */
    {
        CREATE_GDT_SELECTOR_LOW(GDT_PHYS_SRAM_BASE, GDT_PHYS_SRAM_LIMT),
        CREATE_GDT_SELECTOR_HIGH(GDT_PHYS_SRAM_BASE, GDT_PHYS_SRAM_LIMT, 0x9B),
    },

    /* Entry 4: DS: Logical-to-phys while MMU is off, segment selector = 0x0020.  */
    {
        CREATE_GDT_SELECTOR_LOW(GDT_PHYS_SRAM_BASE, GDT_PHYS_SRAM_LIMT),
        CREATE_GDT_SELECTOR_HIGH(GDT_PHYS_SRAM_BASE, GDT_PHYS_SRAM_LIMT, 0x93),
    },


	/* Entry 5: SS for after paging. segment selector = 0x0028*/
    {
        ss_selector_low,
        ss_selector_high,
    },

	/* Entry 6: MMIO segment for after paging. segment selector = 0x0030 */
    {
        CREATE_GDT_SELECTOR_LOW(0x0, MMIO_LIMIT),
        CREATE_GDT_SELECTOR_HIGH(0x0, MMIO_LIMIT,0x93),
    },

	/* Entry 7:  selector for physical access with no segment limit (used from page manager init) segment selector = 0x0038*/
    {

        CREATE_GDT_SELECTOR_LOW(0x0, 0xffffffff),
        CREATE_GDT_SELECTOR_HIGH(0x0, 0xffffffff, 0x93),
    },

    /* Entry 8 (selector=0x0040): TSS Descriptor */
    {        
        tss_selector_low,   
        tss_selector_high,   
    },

};

/* globals */

__stationary_kernel_data__ tGdtHeader _Gdt =
{
    sizeof(tGdtDesc[MAX_GDT_ENTRIES - CONFIG_NUM_GDT_SPARE_ENTRIES]) - 1,
    &_GdtEntries[0]
};

/* physical addresses for startup.s, before segmentation is enabled */
extern unsigned long _GdtEntriesP;
__stationary_kernel_data__ tGdtHeader _GdtPrePaging =
    {
    sizeof(tGdtDesc[MAX_GDT_ENTRIES - CONFIG_NUM_GDT_SPARE_ENTRIES]) - 1,
    (tGdtDesc *) &_GdtEntriesP
    };

/*******************************************************************************
*
* nanoCpuGdtAdd - add a new entry to the GDT
*
* This function is utitlized to append a new entry to the GDT.
*
* RETURNS: Number of bytes used in GDT
*
* \NOMANUAL
*/

__pinned_kernel_code__ unsigned int nanoCpuGdtAdd(
    uint32_t  desc1,           /* 1st half of raw descriptor data */
    uint32_t  desc2            /* 2nd half of raw descriptor data */
    )
{
    uint32_t     *ptr;   /* pointer to raw descriptor data */
    unsigned int  index; /* index into _GdtEntries */

    uint32_t eflags = enter_critical();

    index = (_Gdt.limit + 1) >> 3;           /* Get index from GDT header */

    ptr = (uint32_t *) &_GdtEntries[index];  /* Copy-in raw descriptor data */
    ptr[0] = desc1;
    ptr[1] = desc2;

    _Gdt.limit += sizeof (tGdtDesc);         /* Update GDT limit */

    __asm__ volatile ("lgdt _Gdt");

    leave_critical(eflags);

    return ((index + 1) << 3);
}
