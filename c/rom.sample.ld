MEMORY
{
	AON (rw) : ORIGIN = 0x70000000, LENGTH = 0x10000
	ROM (rx) : ORIGIN = 0x71000000, LENGTH = 0x4000
}

SECTIONS
{
	.vector_table :
	{
		KEEP(*(.vector_table));
	} > ROM

	.text :
	{
		*(.text);
		*(".text.*");
	} > ROM

	.rodata :
	{
		*(.rodata);
		*(".rodata.*");
	} > ROM

	rom_stack_top = ORIGIN(ROM) + LENGTH(ROM) - 12;
}

