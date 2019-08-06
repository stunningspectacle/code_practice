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
#include <clanton_util.h>
#include <ish_sections.h>
//#include <gpio.h>


__init_flow_data__ uint32_t i2c_phys;
#define I2C_PHYS_OFFSET(offset) (i2c_phys + offset)

STATUS ipc_init();

void ipc_isr_handler(void); // TODO move to h file

__init_flow_data__ irq_desc_t system_irqs[] =
{
 //   LEVEL_INTR(I2C0_IRQ,     I2C0_VEC),
 //   LEVEL_INTR(UART1_IRQ,  UART01_VEC),
 //   LEVEL_INTR(I2C1_IRQ,     I2C1_VEC),
 //   LEVEL_INTR(I2C2_IRQ,     I2C2_VEC),
 //   LEVEL_INTR(GPIO_IRQ,     GPIO_VEC),
 //   LEVEL_INTR(IPC_IRQ_HOST2ISH,     IPC_VEC),
 //   LEVEL_INTR(IPC_IRQ_ISH2HOST_CLR, IPC_VEC),
};

__init_flow_data__ unsigned int num_system_irqs = sizeof(system_irqs)/sizeof(system_irqs[0]);

extern void uart_int_isr();
extern void uart_ext_isr();
__init_flow_code__ void registerVecToHandler(void)
{
//	xPortRegisterCInterruptHandler(i2c_isr_bus0, I2C0_VEC);
//	xPortRegisterCInterruptHandler(uart_int_isr, UART01_VEC);
//	xPortRegisterCInterruptHandler(uart_ext_isr,UART_EXT_VEC);
//	xPortRegisterCInterruptHandler(i2c_isr_bus1, I2C1_VEC);
//	xPortRegisterCInterruptHandler(i2c_isr_bus2, I2C2_VEC);
//	xPortRegisterCInterruptHandler(gpio_isr_handler, GPIO_VEC);
//	xPortRegisterCInterruptHandler(ipc_isr_handler, IPC_VEC);
}

__init_flow_code__ void console_init()
{
//    trace_set_i2c_handler(&i2c_dbg_poll_write);
//    trace_init_config();
}


//Initializes all IRQs with IDT vectors (except timer driver which is done in Viper code).
//also configure IRQ agent which will route each PCI device pin to a specific IRQ
__init_flow_code__ void irqs_init()
{
	uint32_t base = MMIO_PCI_ADDRESS(LEGACY_BRIDGE_BUS, LEGACY_BRIDGE_DEVICE, LEGACY_BRIDGE_FUNC, 0);
	uint32_t rcba_base = mem_read(base, LEGACY_BRIDGE_RCBA_OFFSET, 4);

//	rcba_base &= ~(1 << 0); //clear the enable bit
//
//	//map RCBA
//	(void) nanoCpuMmioMap (RCBA_BASE_ADRS, rcba_base + PAGES(3) /* we don't need the first 3 pages */,
//			   RCBA_SIZE, 0);

	// configure agents
    write16(rcba_base + RCBA_IRQAGENT1_OFFSET, IRQ_AGENT1_VAL);
    write16(rcba_base + RCBA_IRQAGENT3_OFFSET, IRQ_AGENT3_VAL);
}


__init_flow_code__ static inline void wait_for_operation_completion()
{
	// poll until stop condition is detected. (we have to ensure the prev operation was completed before we disable i2c)
	while((read32(I2C_PHYS_OFFSET(EARLY_I2C_RAW_INTR_STAT)) & RAW_INTR_STAT_STOP_DET) == 0);
	read32(I2C_PHYS_OFFSET(EARLY_I2C_CLR_STOP_DET)); // clear STOP_DET interrupt
}



__init_flow_code__ void board_init()
{
	uint32_t gba_base, base =0;

	//eeprom_slave_addr = EEPROM_ADDR_GEN_1;

	//find GBA offset

	base = MMIO_PCI_ADDRESS(LEGACY_BRIDGE_BUS, LEGACY_BRIDGE_DEVICE, LEGACY_BRIDGE_FUNC, 0 );
	gba_base = mem_read(base, LEGACY_BRIDGE_GBA_OFFSET, 4);
	gba_base &= 0xffff; //IO port is 16 bit
//	//enable pin 2 to enable level shifter on the board to allow GPIO and UART to get out to the shield
//	outl(
//		inl(gba_base + GBA_EN) | (1 << GBA_LVL_EN_PIN),
//		gba_base + GBA_EN);
//
//	//set as output
//	outl(
//		inl(gba_base + GBA_DIR) & ~(1 << GBA_LVL_EN_PIN),
//		gba_base + GBA_DIR);
//
//	//write 1 to pin
//	outl(
//		inl(gba_base + GBA_OUT_DATA) | (1 << GBA_LVL_EN_PIN),
//		gba_base + GBA_OUT_DATA);


//	pciConfigInLong(
//		LEGACY_BRIDGE_BUS,
//		LEGACY_BRIDGE_DEVICE,
//		LEGACY_BRIDGE_FUNC,
//		LEGACY_BRIDGE_GBA_OFFSET,
//		&gba_base);
//
//	gba_base &= 0xffff; //IO port is 16 bit
//
//	//enable pin 2 to enable level shifter on the board to allow GPIO and UART to get out to the shield
//	io_outLong(
//		io_inLong(gba_base + GBA_EN) | (1 << GBA_LVL_EN_PIN),
//		gba_base + GBA_EN);
//
//	//set as output
//	io_outLong(
//		io_inLong(gba_base + GBA_DIR) & ~(1 << GBA_LVL_EN_PIN),
//		gba_base + GBA_DIR);
//
//	//write 1 to pin
//	io_outLong(
//		io_inLong(gba_base + GBA_OUT_DATA) | (1 << GBA_LVL_EN_PIN),
//		gba_base + GBA_OUT_DATA);


	//configure extender chip to give up control over I2C, GPIO, UART
	//pciDevFind(I2C_BDF, I2C_BAR, &i2c_phys,&size,&irq_pin);
	base = mem_read(I2C_BDF, 0x10, 4);
	i2c_phys = base;

	//map early I2C
	//(void) nanoCpuMmioMap (EARLY_I2C_BASE_ADRS, i2c_phys, EARLY_I2C_SIZE, 0);


	//configure I2C controller:
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x0); // disable i2c
	write32(I2C_PHYS_OFFSET(EARLY_I2C_INTR_MASK),0x0); // disable interrupts
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CON), EARLY_I2C_GEN2_CONF);

	// configure thresholds to 0 zero since we work in byte resolution
	write32(I2C_PHYS_OFFSET(EARLY_I2C_RX_TL),0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_TX_TL),0);

	// configure EXP2 mux (enable I2C):
	write32(I2C_PHYS_OFFSET(EARLY_I2C_TAR), GEN2_IO_EXP2_SLAVE_ADDR);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x1);

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), IO_EXP_OUT_REG_1);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0xEF);

	wait_for_operation_completion();

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), IO_EXP_CON_REG_1);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0xEF);

	wait_for_operation_completion();

	// configure EXP0 mux (enable UART Tx, GPIO pin 5):
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_TAR), GEN2_IO_EXP0_SLAVE_ADDR);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x1);

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), IO_EXP_OUT_REG_0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), 0x1);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0xAA);

	wait_for_operation_completion();

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), IO_EXP_CON_REG_0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), 0xFE);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0xAA);

	wait_for_operation_completion();

	// configure EXP1 mux (enable UART Rx, GPIO pin 6):
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_TAR), GEN2_IO_EXP1_SLAVE_ADDR);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x1);

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), IO_EXP_OUT_REG_0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), 0x5);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0x70);

	wait_for_operation_completion();

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), IO_EXP_CON_REG_0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), 0xFA);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0x8F);

	wait_for_operation_completion();

	//configure PCA9685 mux (SPI):
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x0);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_TAR), GEN2_PCA9685_SLAVE_ADDR);
	write32(I2C_PHYS_OFFSET(EARLY_I2C_EN),0x1);

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), 0x31); // configure LED10 (to control MUX6_SEL)
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0x10); //  configure full off (SS line)

	wait_for_operation_completion();

	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), 0x29); // configure LED8 (to control MUX4_SEL)
	write32(I2C_PHYS_OFFSET(EARLY_I2C_CMD), CMD_STOP|0x10); // configure full off (MOSI line)

	wait_for_operation_completion();
}

extern void uart_drv_init();
// Creates all system resources and initializes HW
__init_flow_code__ void initHW( void )
{
//	irqs_init();
//	board_init();
//	i2c_init();
	uart_drv_init();
//	gpio_init();
//	ipc_init();
//	hpet_init();
}




