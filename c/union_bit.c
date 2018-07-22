#include <stdio.h>

typedef unsigned int uint32_t;
typedef union {
	unsigned int raw;
	struct {
		unsigned int bit0: 1;
		unsigned int bit1: 1;
		unsigned int bit2: 1;
		unsigned int bit3: 1;
		unsigned int bit4: 1;
		unsigned int bit5: 1;
		unsigned int bit6_15: 10;
		unsigned int bit16_28: 14;
		//union {
			unsigned int bit29: 1;
			//unsigned int bit2k: 1;
		//};
		unsigned int bit30: 1;
		unsigned int bit31: 1;
	};
} testbit_t;

union dbg_data {
	uint32_t raw_data;
	struct {
		uint32_t reserved1         : 15; //Bit0-14
		uint32_t dtf_trace_enable  : 1; //Bit15
		uint32_t bup_sleep_disable : 1; //Bit16
		uint32_t i2c_address       : 7; //BIT17-23
		uint32_t i2c_port          : 2; //BIT24-25
		uint32_t dma_trace_enable  : 1; //BIT26:
		uint32_t i2c_dbg_enable    : 1; //BIT27:
		uint32_t host_dbg_enable   : 1; //BIT28:
		uint32_t lpk_trace_enable  : 1; //BIT29:
		uint32_t i2c_trace_enable  : 1; //BIT30:
		uint32_t host_trace_enable : 1; //BIT31:
	};
};

union dbg_data2 {
	uint32_t raw_data;
	struct {
		uint32_t reserved1         : 16; //Bit0-15
		uint32_t bup_sleep_disable : 1; //Bit16
		uint32_t i2c_address       : 7; //BIT17-23
		uint32_t i2c_port          : 2; //BIT24-25
		uint32_t dma_trace_enable  : 1; //BIT26:
		uint32_t i2c_dbg_enable    : 1; //BIT27:
		uint32_t host_dbg_enable   : 1; //BIT28:
		union {
			uint32_t lpk_trace_enable : 1; //BIT29:
			uint32_t dtf_trace_enable : 1;
		};
		uint32_t i2c_trace_enable  : 1; //BIT30:
		uint32_t host_trace_enable : 1; //BIT31:
	};
};

int main()
{
	union dbg_data data;
	union dbg_data2 data2;

	printf("sizeof(data): %d, sizeof(data2): %d\n", sizeof(data), sizeof(data2));
	data.raw_data = (1 << 30);
	printf("i2c trace: %d\n", data.i2c_trace_enable);

	data2.raw_data = (1 << 30);
	printf("i2c trace: %d\n", data2.i2c_trace_enable);

	return 0;
} 

/*
void main(void)
{
	testbit_t mytest;

	mytest.raw = (1 << 30);

	printf("bit 29: %d\n", mytest.bit29);
	//printf("bit 2k: %d\n", mytest.bit2k);
	printf("bit 30: %d\n", mytest.bit30);
	printf("bit 31: %d\n", mytest.bit31);
	printf("0x%x\n", mytest.raw);
}

*/
