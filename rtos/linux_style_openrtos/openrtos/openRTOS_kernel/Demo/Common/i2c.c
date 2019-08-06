/*
 * i2c.c
 *
 *  Created on: May 10, 2015
 *      Author: abrill
 */

#include <mmio.h>
#include <reg_rw.h>
#include <string.h>

/* See file REFERENCES for more details. */

/* [I2C], 6.3.28 IC_ENABLE */
#define BIT(b)                  ( 1 << (b) )
#define I2C_IC_ENABLE           BIT(0)

/* [I2C], 6.3.14 IC_RAW_INTR_STAT */
#define I2C_TX_EMPTY            BIT(4)


/* [I2C], 6.3.5 IC_DATA_CMD */
#define I2C_CMD_STOP            BIT(9)
#define I2C_CMD_SHIFT           8
#define I2C_CMD_WRITE           0
#define I2C_CMD_READ            1


/* [I2C], 6.3.1 IC_CON */
#define I2C_MASTER_MODE         BIT(0)
#define I2C_SPEED(mode)         ( (mode) << 1 )
#define I2C_IC_RESTART_EN       BIT(5)
#define I2C_IC_SLAVE_DISABLE    BIT(6)

#define I2C_STANDARD_MODE       1
#define I2C_FAST_MODE           2
#define I2C_HIGH_SPEED_MODE     3


#define I2C_WRITE_COMMAND       ( (I2C_CMD_WRITE) << I2C_CMD_SHIFT )
#define I2C_READ_COMMAND        ( (I2C_CMD_READ) << I2C_CMD_SHIFT )


#define I2C_TIMEOUT 100000 // 1ms for 100MHz
unsigned int i2c_base = I2C0_BASE;

void i2c_write_char(const int character)
{
	write32(I2C_DATA_CMD(i2c_base), (unsigned char)character | I2C_WRITE_COMMAND | I2C_CMD_STOP);


	while (!(read32(I2C_RAW_INTR_STAT(i2c_base)) & I2C_TX_EMPTY));

	/* NOTE: there is NO recursion here! */
	if (character == '\n')
		i2c_write_char('\r');
}

void i2c_out(char* str)
{
	int len = strlen(str);
	int i = 0;

	write32(I2C_ENABLE(i2c_base), I2C_IC_ENABLE);

	for (i = 0; i < len; i++)
	{
		i2c_write_char(str[i]);
	}


	write32(I2C_ENABLE(i2c_base), 0);
}


void i2clib_start(void)
{
	unsigned char slave_addr = 0x5E;
    write32(I2C_ENABLE(i2c_base), 0);

    /* NOTE: I do not set I2C_IC_RESTART_EN, see [I2C], 6.3.1 IC_CON.
     *       Seems we do not need any of its functionality in ROM.
     * Vladik, 1.01.2013
     */
    write32(I2C_CON(i2c_base), I2C_MASTER_MODE |
                        I2C_SPEED(I2C_FAST_MODE) |
                        I2C_IC_SLAVE_DISABLE);

    write32(I2C_TAR(i2c_base), slave_addr);
    write32(I2C_ENABLE(i2c_base), I2C_IC_ENABLE);
}


