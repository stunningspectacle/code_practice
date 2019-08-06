/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2012 - 2014 Intel Corporation. All Rights Reserved.

   The source code contained or described herein and all documents related
   to the source code ("Material") are owned by Intel Corporation or its
   suppliers or licensors. Title to the Material remains with Intel Corporation
   or its suppliers and licensors. The Material contains trade secrets and
   proprietary and confidential information of Intel or its suppliers and
   licensors. The Material is protected by worldwide copyright and trade secret
   laws and treaty provisions. No part of the Material may be used, copied,
   reproduced, modified, published, uploaded, posted, transmitted, distributed,
   or disclosed in any way without Intel's prior express written permission.

   No license under any patent, copyright, trade secret or other intellectual
   property right is granted to or conferred upon you by disclosure or delivery
   of the Materials, either expressly, by implication, inducement, estoppel or
   otherwise. Any license under such intellectual property rights must be
   express and approved by Intel in writing.
 --*/

#include <sys/types.h>
#include <stdio.h>
#include <mmio.h>
#include <reg_rw.h>
#include <ia32_interrupts.h>

#define DEST_APIC_ID            0

void set_ioapic_redtbl_raw(const unsigned irq, const unsigned int val);

__pinned_kernel_code__ void write_ioapic_reg(const off_t reg, const unsigned int val)
{
    write32(IOAPIC_IDX, (unsigned char)reg);
    write32(IOAPIC_WDW, val);
}

__pinned_kernel_code__ unsigned int read_ioapic_reg(const off_t reg)
{
    write32(IOAPIC_IDX, (unsigned char)reg);
    return read32(IOAPIC_WDW);
}

__pinned_kernel_code__ void set_ioapic_redtbl(irq_desc_t irq_desc)
{
    unsigned int val = irq_desc.vector	|
            IOAPIC_REDTBL_DELMOD_FIXED  |
            IOAPIC_REDTBL_DESTMOD_PHYS  |
            		   irq_desc.polarity|
            		   irq_desc.trigger;

    set_ioapic_redtbl_raw(irq_desc.irq, val);
}

__pinned_kernel_code__ void set_ioapic_redtbl_raw(const unsigned irq, const unsigned int val)
{
    /* [IOAPIC], 3.2.4. "IOREDTBL[23:0] I/O REDIRECTION TABLE REGISTERS" */
    const off_t                 redtbl_lo = IOAPIC_IOREDTBL + 2 * irq;
    const off_t                 redtbl_hi = redtbl_lo + 1;

    write_ioapic_reg(redtbl_lo, val);
    write_ioapic_reg(redtbl_hi, DEST_APIC_ID);
}

//Maintains the current IRQs that are enabled in IOAPIC (bitmask).
__pinned_kernel_bss__ uint64_t ioapic_mask = 0;

__pinned_kernel_code__ void unmask_interrupt(unsigned int irq)
{
	const off_t redtbl_lo = IOAPIC_IOREDTBL + 2 * irq;
	unsigned int val = read_ioapic_reg(redtbl_lo);
	val &= ~IOAPIC_REDTBL_MASK;
	set_ioapic_redtbl_raw(irq, val);
    ioapic_mask |= ((uint64_t)1) << irq;
}

__pinned_kernel_code__ void mask_interrupt(unsigned int irq)
{
	const off_t redtbl_lo = IOAPIC_IOREDTBL + 2 * irq;
	unsigned int val = read_ioapic_reg(redtbl_lo);
	val |= IOAPIC_REDTBL_MASK;
	set_ioapic_redtbl_raw(irq, val);
    ioapic_mask &= ~(((uint64_t)1) << irq);
}

__init_flow_code__  void initIOApic(void)
{
    const unsigned              max_entries = (read_ioapic_reg(IOAPIC_VERSION) >> 16) & 0xff;
    unsigned                    entry;

    for (entry = 0; entry < max_entries; entry++)
    {
        set_ioapic_redtbl_raw(entry, IOAPIC_REDTBL_MASK);
    }

    for (entry = 0; entry < num_system_irqs; entry++)
        {
            set_ioapic_redtbl_raw(system_irqs[entry].irq,
            				  system_irqs[entry].vector      |
                              IOAPIC_REDTBL_DELMOD_FIXED  |
                              IOAPIC_REDTBL_DESTMOD_PHYS  |
                              IOAPIC_REDTBL_MASK  |
                              system_irqs[entry].polarity    |
                              system_irqs[entry].trigger);
        }

}
