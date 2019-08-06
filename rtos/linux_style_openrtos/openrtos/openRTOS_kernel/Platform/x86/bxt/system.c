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
#include "trace.h"
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
void spi0_isr (void);
void spi1_isr (void);
void wdt_isr_handler(void);
void wdt_init();
void d3_bme_isr();

__init_flow_data__ irq_desc_t system_irqs[] =
{
    LEVEL_INTR(I2C0_IRQ,     		 I2C0_VEC),
    LEVEL_INTR(I2C1_IRQ,     		 I2C1_VEC),
    LEVEL_INTR(I2C2_IRQ,     		 I2C2_VEC),
    LEVEL_INTR(GPIO_IRQ,     		 GPIO_VEC),
    LEVEL_INTR(IPC_IRQ_HOST2ISH,     IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2HOST_CLR, IPC_VEC),
    LEVEL_INTR(IPC_IRQ_AUDIO2ISH,    IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2AUDIO_CLR,IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISP2ISH,      IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2ISP_CLR,  IPC_VEC),
    LEVEL_INTR(IPC_IRQ_SEC2ISH,		 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_PMC2ISH, 	 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2SEC_CLR,  IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISH2PMC_CLR,  IPC_VEC),
    LEVEL_INTR(IPC_IRQ_CSE_CSR, 	 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_PMC_CSR, 	 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_AUDIO_CSR, 	 IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISP_CSR, 	 IPC_VEC),
    LEVEL_INTR(DMA_IRQ,       		 DMA_VEC),
    LEVEL_INTR(HPET_TIMER0_IRQ,      HPET_TIMER0_VEC),
    LEVEL_INTR(HPET_TIMER1_IRQ,      HPET_TIMER12_VEC),
    LEVEL_INTR(HPET_TIMER2_IRQ,      HPET_TIMER12_VEC),
    LEVEL_INTR(UART0_IRQ,  UART0_VEC),
    LEVEL_INTR(UART1_IRQ,  UART1_VEC),
    LEVEL_INTR(SPI0_IRQ,      SPI0_VEC),
    LEVEL_INTR(SPI1_IRQ,      SPI1_VEC),
    LEVEL_INTR(WDT_IRQ,      WDT_VEC),
    LEVEL_INTR(PMU_WAKEUP_IRQ,      PMU_WAKEUP_VEC),
    LEVEL_INTR(D3_RISE_IRQ,     D3_BME_VEC),
    LEVEL_INTR(D3_FALL_IRQ,     D3_BME_VEC),
    LEVEL_INTR(BME_RISE_IRQ,    D3_BME_VEC),
    LEVEL_INTR(BME_FALL_IRQ,    D3_BME_VEC),
};

__init_flow_data__ unsigned int num_system_irqs = sizeof(system_irqs)/sizeof(system_irqs[0]);


__init_flow_code__ void registerVecToHandler(void)
{
	xPortRegisterCInterruptHandler(i2c_isr_bus0, I2C0_VEC);
	xPortRegisterCInterruptHandler(i2c_isr_bus1, I2C1_VEC);
	xPortRegisterCInterruptHandler(i2c_isr_bus2, I2C2_VEC);
	xPortRegisterCInterruptHandler(gpio_isr_handler, GPIO_VEC);
	xPortRegisterCInterruptHandler(ipc_isr_hdlr,  IPC_VEC);
	xPortRegisterCInterruptHandler(dma_isr_handler,  DMA_VEC);
	//xPortRegisterCInterruptHandler(hpet_isr_handler,  HPET_TIMER0_VEC);
	xPortRegisterCInterruptHandler(hpet_isr_secondary_handler,  HPET_TIMER12_VEC);
	xPortRegisterCInterruptHandler(uart_isr0,  UART0_VEC);
	xPortRegisterCInterruptHandler(uart_isr1,  UART1_VEC);
	xPortRegisterCInterruptHandler(spi0_isr,  SPI0_VEC);
	xPortRegisterCInterruptHandler(spi1_isr,  SPI1_VEC);
	xPortRegisterCInterruptHandler(wdt_isr_handler,  WDT_VEC);
    xPortRegisterCInterruptHandler(d3_bme_isr,  D3_BME_VEC);
}

// Creates all system resources and initializes HW
__init_flow_code__ void initHW( void )
{
	dma_drv_init();
	i2c_init();
	gpio_init();
	ipc_init();
	uart_drv_init();
	spi_drv_init();
	wdt_init();
	// hpet initialization moved to xPortStartScheduler function because it should start before the scheduler
}

__init_flow_code__ void console_init()
{
    trace_set_i2c_handler(&i2c_dbg_poll_write);
	trace_set_uart_handler(&uart_dbg_poll_write);
    trace_init_config();
}



#if ISH_CONFIG_SUPPORT_PAGING
__init_flow_code__ void mmapHW( void )
{

    mmuMap(LAPIC_BASE,  LAPIC_BASE_PHYS,   LOAPIC_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(IOAPIC_BASE, IOAPIC_BASE_PHYS,  IOAPIC_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(I2C0_BASE,   I2C0_BASE_PHYS,    I2C0_SIZE,    MMIO_PTE_FLAGS);
    mmuMap(I2C1_BASE,   I2C1_BASE_PHYS,    I2C1_SIZE,    MMIO_PTE_FLAGS);
    mmuMap(I2C2_BASE,   I2C2_BASE_PHYS,    I2C2_SIZE,    MMIO_PTE_FLAGS);
    mmuMap(SPI0_BASE,   SPI0_SC_BASE_PHYS, SPI0_SC_SIZE, MMIO_PTE_FLAGS);
    mmuMap(SPI1_BASE,   SPI1_SC_BASE_PHYS, SPI1_SC_SIZE, MMIO_PTE_FLAGS);
    mmuMap(HPET_BASE,   HPET_BASE_PHYS,    HPET_SIZE,    MMIO_PTE_FLAGS);
    mmuMap(IPC_BASE,    IPC_BASE_PHYS,     IPC_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(HSU_BASE,    HSU_BASE_PHYS,     HSU_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(CCU_BASE,    CCU_BASE_PHYS,     CCU_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(PMU_BASE,    PMU_BASE_PHYS,     PMU_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(OCP_BASE,    OCP_BASE_PHYS,     OCP_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(GPIO_BASE,   GPIO_BASE_PHYS,    GPIO_SIZE,    MMIO_PTE_FLAGS);
    mmuMap(DMA_BASE,    DMA_BASE_PHYS,     DMA_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(MISC_BASE,   MISC_BASE_PHY,     MISC_SIZE,    MMIO_PTE_FLAGS);
    mmuMap(WDT_BASE,    WDT_BASE_PHYS,     WDT_SIZE,     MMIO_PTE_FLAGS);
    mmuMap(SBEP_BASE,   SBEP_BASE_PHYS,    SBEP_BASE_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(SRAM_CTRL_BASE, SRAM_CTRL_BASE_PHYS, SRAM_CTRL_SIZE,  MMIO_PTE_FLAGS);

    mmuMap(LPK_BASE, LPK_BASE_PHYS, LPK_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(LDO_BASE, LDO_BASE_PHYS, LDO_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(ILB_BASE, ILB_BASE_PHYS, ILB_POOL_SIZE,  MMIO_PTE_FLAGS);
}
#endif

