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
#include "ish_sections.h"
#include "uart_dbg.h"


void ipc_isr_hdlr (void);
void ipc_init();
void hpet_drv_init(void);
void uart_drv_init(void);
void uart_isr0(void);
void uart_isr1(void);
void uart_isr2(void);
void spi_drv_init ();
void spi0_isr (void);
void spi1_isr (void);
void wdt_isr_handler(void);
void wdt_init();
void d3_bme_isr();
void pm_reset_prep_isr();
void fabric_isr();

__init_flow_data__ irq_desc_t system_irqs[] =
{
    LEVEL_INTR(I2C0_IRQ,            I2C0_VEC),
    LEVEL_INTR(I2C1_IRQ,            I2C1_VEC),
    LEVEL_INTR(I2C2_IRQ,            I2C2_VEC),

    LEVEL_INTR(SPI0_IRQ,            SPI0_VEC),
    LEVEL_INTR(SPI1_IRQ,            SPI1_VEC),
    LEVEL_INTR(UART0_IRQ,           UART0_VEC),
    LEVEL_INTR(UART1_IRQ,           UART1_VEC),
    LEVEL_INTR(UART2_IRQ,           UART2_VEC),

    LEVEL_INTR(D3_RISE_IRQ,         D3_BME_VEC),
    LEVEL_INTR(RESET_PREP_IRQ,      RESET_PREP_VEC),
    LEVEL_INTR(PMU_WAKEUP_IRQ,      PMU_WAKEUP_VEC),


    LEVEL_INTR(WDT_IRQ,             WDT_VEC),
    LEVEL_INTR(DMA_IRQ,             DMA_VEC),
    LEVEL_INTR(GPIO_IRQ,            GPIO_VEC),
    LEVEL_INTR(SBEP_IRQ,            SBEP_VEC),

    LEVEL_INTR(IPC_IRQ_HOST2ISH,    IPC_VEC),
    LEVEL_INTR(IPC_IRQ_SEC2ISH,     IPC_VEC),
    LEVEL_INTR(IPC_IRQ_PMC2ISH,     IPC_VEC),
    LEVEL_INTR(IPC_IRQ_AUDIO2ISH,   IPC_VEC),
    LEVEL_INTR(IPC_IRQ_ISP2ISH,     IPC_VEC),

    LEVEL_INTR(HPET_TIMER0_IRQ,     HPET_TIMER0_VEC),
    LEVEL_INTR(FABRIC_IRQ,          FABRIC_VEC),
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
    // xPortRegisterCInterruptHandler(hpet_isr_handler,  HPET_TIMER0_VEC);
    // xPortRegisterCInterruptHandler(hpet_isr_secondary_handler,  HPET_TIMER12_VEC);
    xPortRegisterCInterruptHandler(uart_isr0,  UART0_VEC);
    xPortRegisterCInterruptHandler(uart_isr1,  UART1_VEC);
    xPortRegisterCInterruptHandler(uart_isr2,  UART2_VEC);
    xPortRegisterCInterruptHandler(spi0_isr,  SPI0_VEC);
    xPortRegisterCInterruptHandler(spi1_isr,  SPI1_VEC);
    xPortRegisterCInterruptHandler(wdt_isr_handler,  WDT_VEC);
    xPortRegisterCInterruptHandler(d3_bme_isr,  D3_BME_VEC);
    xPortRegisterCInterruptHandler(pm_reset_prep_isr, RESET_PREP_VEC);
    xPortRegisterCInterruptHandler(fabric_isr,  FABRIC_VEC);
}

__init_flow_code__ void console_init()
{
    trace_set_i2c_handler(&i2c_dbg_poll_write);
	trace_set_uart_handler(&uart_dbg_poll_write);
    trace_init_config();
}

#define DLL 0x0
#define DLH 0x4
#define LCR 0xC
#define DLF 0xC0

#if defined(FPGA)
#define PM_UART_HSU_IDLE_DLL_115200 0x0A
#define PM_UART_HSU_IDLE_DLF_115200 0x0D
#else
#define PM_UART_HSU_IDLE_DLL_115200 0x36
#define PM_UART_HSU_IDLE_DLF_115200 0x04
#endif

__pinned_kernel_code__ static void uart_to_idle()
{
	write32(UART0_BASE + LCR, 0x80);
	write32(UART0_BASE + DLL, PM_UART_HSU_IDLE_DLL_115200);
	write32(UART0_BASE + DLH, 0);
	write32(UART0_BASE + DLF, PM_UART_HSU_IDLE_DLF_115200);
	write32(UART0_BASE + LCR, 0);

	write32(UART1_BASE + LCR, 0x80);
	write32(UART1_BASE + DLL, PM_UART_HSU_IDLE_DLL_115200);
	write32(UART1_BASE + DLH, 0);
	write32(UART1_BASE + DLF, PM_UART_HSU_IDLE_DLF_115200);
	write32(UART1_BASE + LCR, 0);

	write32(UART2_BASE + LCR, 0x80);
	write32(UART2_BASE + DLL, PM_UART_HSU_IDLE_DLL_115200);
	write32(UART2_BASE + DLH, 0);
	write32(UART2_BASE + DLF, PM_UART_HSU_IDLE_DLF_115200);
	write32(UART2_BASE + LCR, 0);
}

// Creates all system resources and initializes HW
__init_flow_code__ void initHW( void )
{
	i2c_init();
	gpio_init();
	ipc_init();
	dma_drv_init();
	uart_drv_init();
	uart_to_idle();
	spi_drv_init();
	//hpet_drv_init();
	wdt_init();

    Enable_Interrupt(FABRIC_IRQ);
}

#if ISH_CONFIG_SUPPORT_PAGING
__init_flow_code__ void mmapHW( void )
{
    mmuMap(LAPIC_BASE,	LAPIC_BASE_PHYS,    LOAPIC_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(IOAPIC_BASE,	IOAPIC_BASE_PHYS,   IOAPIC_SIZE,  MMIO_PTE_FLAGS);

    mmuMap(I2C0_BASE,   I2C0_BASE_PHYS,     I2C0_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(I2C1_BASE,   I2C1_BASE_PHYS,     I2C1_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(I2C2_BASE,   I2C2_BASE_PHYS,     I2C2_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(SPI0_BASE,   SPI0_SC_BASE_PHYS,  SPI0_SC_SIZE, MMIO_PTE_FLAGS);
    mmuMap(SPI1_BASE,   SPI1_SC_BASE_PHYS,  SPI1_SC_SIZE, MMIO_PTE_FLAGS);
    mmuMap(HPET_BASE,   HPET_BASE_PHYS,     HPET_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(IPC_BASE,    IPC_BASE_PHYS,      IPC_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(UART0_BASE,  UART0_BASE_PHYS,    UART0_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(UART1_BASE,  UART1_BASE_PHYS,    UART1_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(UART2_BASE,  UART2_BASE_PHYS,    UART2_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(CCU_BASE,    CCU_BASE_PHYS,      CCU_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(PMU_BASE,    PMU_BASE_PHYS,      PMU_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(SBEP_BASE,   SBEP_BASE_PHYS,     SBEP_BASE_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(SRAM_CTRL_BASE, SRAM_CTRL_BASE_PHYS, SRAM_CTRL_SIZE, MMIO_PTE_FLAGS);

    mmuMap(GPIO_BASE,   GPIO_BASE_PHYS,     GPIO_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(DMA_BASE,    DMA_BASE_PHYS,      DMA_SIZE,   MMIO_PTE_FLAGS);
    mmuMap(MISC_BASE,   MISC_BASE_PHY,      MISC_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(LDO_BASE,    LDO_BASE_PHYS,      LDO_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(DTF_BASE,    DTF_BASE_PHYS,      DTF_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(WDT_BASE,    WDT_BASE_PHYS,      WDT_SIZE,  MMIO_PTE_FLAGS);
    mmuMap(HBW_FABRIC_BASE, HBW_FABRIC_BASE_PHYS, HBW_FABRIC_SIZE, MMIO_PTE_FLAGS);
    mmuMap(PER0_FABRIC_BASE, PER0_FABRIC_BASE_PHYS, PER0_FABRIC_SIZE, MMIO_PTE_FLAGS);
}
#endif

