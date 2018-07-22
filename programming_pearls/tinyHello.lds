ENTRY(nomain)

SECTIONS
{
	. = 0x300000 + SIZEOF_HEADERS;
	.tinytext : { *(.text) *(.data) *(.rodata) }
   	/DISCARD/ : { *(.comment) }
}
