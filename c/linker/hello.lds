/* ENTRY(mymain); */

SECTIONS
{
	START_ADD = 0xbad000;

	. = START_ADD; /* default start address is 0x0 */

/*
	.text : {
		*(.text);
	}
*/
}
