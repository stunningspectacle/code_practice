/*
 * openrtos_drivers.c
 *
 *  Created on: Jun 16, 2015
 *      Author: abrill
 */
#include <ia32_interrupts.h>
#include <ish_i2c.h>
#include <os_api.h>
#include <reg_rw.h>
#include <ish_hw.h>
#include <gpio.h>
#include <dma.h>
#include <ish_ipc.h>
#include <mmu.h>
#include <trace.h>
#include <ish_sections.h>
#include "uart_dbg.h"

void ipc_isr_hdlr (void);
void ipc_init();
void hpet_isr_handler(void);
void hpet_isr_secondary_handler(void);
void hpet_drv_init(void);
void uart_drv_init();
void uart_isr0(void);
void uart_isr1(void);
void spi_drv_init ();
void spi_isr_hdlr (void);
void wdt_isr_handler(void);
void wdt_init();

__init_flow_data__ irq_desc_t system_irqs[] =
{
    LEVEL_INTR(I2C0_IRQ,     		 I2C0_VEC),
    LEVEL_INTR(I2C1_IRQ,     		 I2C1_VEC),
    LEVEL_INTR(GPIO_IRQ,     		 GPIO_VEC),
    LEVEL_INTR(IPC_IRQ_HOST2ISH,     IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2HOST_CLR, IPC_VEC),
    LEVEL_INTR(IPC_IRQ_SEC2ISH,		 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_PMC2ISH, 	 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2SEC_CLR,  IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2PMC_CLR,  IPC_VEC),
    LEVEL_INTR(DMA_IRQ,       		 DMA_VEC),
    LEVEL_INTR(HPET_TIMER0_IRQ,      HPET_TIMER0_VEC),
    LEVEL_INTR(HPET_TIMER1_IRQ,      HPET_TIMER12_VEC),
    LEVEL_INTR(HPET_TIMER2_IRQ,      HPET_TIMER12_VEC),
    LEVEL_INTR(WDT_IRQ,      WDT_VEC),
};

__init_flow_code__ void console_init()
{
    trace_set_i2c_handler(&i2c_dbg_poll_write);
	trace_set_uart_handler(&uart_dbg_poll_write);
    trace_init_config();
}

__init_flow_data__ unsigned int num_system_irqs = sizeof(system_irqs)/sizeof(system_irqs[0]);


__init_flow_code__ void registerVecToHandler(void)
{
	xPortRegisterCInterruptHandler(i2c_isr_bus0, I2C0_VEC);
	xPortRegisterCInterruptHandler(i2c_isr_bus1, I2C1_VEC);
	xPortRegisterCInterruptHandler(gpio_isr_handler, GPIO_VEC);
	xPortRegisterCInterruptHandler(ipc_isr_hdlr,  IPC_VEC);
	xPortRegisterCInterruptHandler(dma_isr_handler,  DMA_VEC);
	//xPortRegisterCInterruptHandler(hpet_isr_handler,  HPET_TIMER0_VEC);
	xPortRegisterCInterruptHandler(hpet_isr_secondary_handler,  HPET_TIMER12_VEC);
	xPortRegisterCInterruptHandler(wdt_isr_handler,  WDT_VEC);
}

// Creates all system resources and initializes HW
__init_flow_code__ void initHW( void )
{
	i2c_init();
	gpio_init();
	ipc_init();
	dma_drv_init();
	//uart_drv_init();
	//spi_drv_init();
	//hpet_drv_init();
	wdt_init();
}

#if ISH_CONFIG_SUPPORT_PAGING
__init_flow_code__ void mmapHW( void )
{
		mmuMap(I2C0_BASE ,I2C0_BASE_PHYS 			   ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(SPI0_BASE ,SPI0_SC_BASE_PHYS 		   ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(I2C1_BASE ,I2C1_BASE_PHYS               ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(HSU_BASE, HSU_BASE_PHYS                 ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(SPI1_BASE ,SPI1_SC_BASE_PHYS            ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(I2C2_BASE ,I2C2_BASE_PHYS               ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(LPK_BASE  ,LPK_BASE_PHYS	               ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(GPIO_BASE ,GPIO_BASE_PHYS               ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(DMA_BASE  ,DMA_BASE_PHYS                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(SRAM_CTRL_BASE ,SRAM_CTRL_BASE_PHYS     ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(OCP_BASE  ,OCP_BASE_PHYS                 ,10*MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(PMU_BASE  ,PMU_BASE_PHYS                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(CCU_BASE  ,CCU_BASE_PHYS                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(IPC_BASE  ,IPC_BASE_PHYS                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(LDO_BASE  ,LDO_BASE_PHYS                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(MISC_BASE ,MISC_BASE_PHY                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(SBEP_BASE  ,SBEP_BASE_PHYS              ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(WDT_BASE  ,WDT_BASE_PHYS                ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(ILB_BASE   ,ILB_BASE_PHYS               ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(LAPIC_BASE ,LAPIC_BASE_PHYS             ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(IOAPIC_BASE ,IOAPIC_BASE_PHYS           ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(HPET_BASE ,HPET_BASE_PHYS               ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
		mmuMap(WDT_BASE, WDT_BASE_PHYS                 ,MMU_PAGE_SIZE,  MMU_ENTRY_PRESENT | MMU_ENTRY_WRITE  );
}
#endif

