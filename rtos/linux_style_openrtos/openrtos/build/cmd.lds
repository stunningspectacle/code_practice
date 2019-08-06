LD_FEATURE("SANE_EXPR");

ENTRY(_start_freertos);
#include <ish_config.h>
#include <linker-defs-ish-sections.h>
#include <linker-defs-openrtos-sections.h>
#include <mmio_phys.h>

#define GDT_DATA_FLAGS 0x93
#define GDT_CODE_FLAGS 0x9B

/* 0:15 - limit(low). 16:31 - base(low) */
#define CREATE_GDT_SELECTOR_LOW(base,limit,flags) (((ABSOLUTE(limit) >> 12) & 0xFFFF) | ((ABSOLUTE(base) & 0xFFFF) << 16 ))

/* 0:7 - base (mid). 8-15 - flags. 16-23 - limit(high). 24-31 - base(high). */
#define CREATE_GDT_SELECTOR_HIGH(base,limit,flags)	(((ABSOLUTE(base)  >> 16) & 0xFF) | ((ABSOLUTE(flags) << 8)  & 0xFF00) |  ((ABSOLUTE(limit) >> 12) & 0xFF0000) |  ((ABSOLUTE(base))  & 0xFF000000) | 0xC00000)

/* This script classifies various sections of ISH in 5 general categories as follows:

    --------------------
    |    boot code (aligned to start)
    --------------------
    |    boot data
    --------------------
    |    system stack 
    --------------------
    |    code
    --------------------
    |    data 
    --------------------
    |    bss 
    --------------------

 *  Boot sections (code & data) are loaded to SRAM first, before MMU paging enabled and the other sections are loaded when needed by the Demand Paging hook. 
 ** Every section below defines two linker symbols - *_section_start and *_section_end - for the MMU init flow that maps all sections in the page table.

*/    

#if !ISH_CONFIG_ENFORCE_SEGMENTS_LIMIT
	__cs_selector_low  = CREATE_GDT_SELECTOR_LOW(0,0xFFFFFFFF,GDT_CODE_FLAGS);
	__cs_selector_high = CREATE_GDT_SELECTOR_HIGH(0,0xFFFFFFFF,GDT_CODE_FLAGS);
	__ss_selector_low  = CREATE_GDT_SELECTOR_LOW(0,0xFFFFFFFF,GDT_DATA_FLAGS);
	__ss_selector_high = CREATE_GDT_SELECTOR_HIGH(0,0xFFFFFFFF,GDT_DATA_FLAGS);
	__ds_selector_low  = CREATE_GDT_SELECTOR_LOW(0,0xFFFFFFFF,GDT_DATA_FLAGS);
	__ds_selector_high = CREATE_GDT_SELECTOR_HIGH(0,0xFFFFFFFF,GDT_DATA_FLAGS);
#endif
	
SECTIONS
{
    . = ISH_BOOT_START;

    .boot_code_section : {
        __bootcode_start = .;
        
        __init_code_start = .; 
        *(.entrypoint)
        *(.text.ish_defined_section.init_flow*)
        . = ALIGN(0x1000);
        __init_code_end = .;
    
        __stat_code_start = .;
        *(.boot_code.ish_stationary_section*)
        __stat_code_end = .;
        
        __pinned_code_start = .;
        PINNED_OBJECTS_TEXT
        PINNED_OBJECTS_RODATA
        OS_PINNED_OBJECTS_TEXT
        OS_PINNED_OBJECTS_RODATA
        *(.boot_code.ish_pinned_section*)
        __pinned_code_end = .;
        __bootcode_end = ALIGN(0x1000);
#if ISH_CONFIG_SUPPORT_CCM
	// in CCM memory, Data and code must reside in seperate banks
        __bootcode_initial_load_end = ALIGN(ISH_CONFIG_SRAM_BANK_SIZE);
#else
       __bootcode_initial_load_end = __bootcode_end;
#endif
    }

    .boot_data_section : {
#if ISH_CONFIG_SUPPORT_DEMAND_PAGING
		__bootdata_start = .;
#else //no paging. we want bringup/isha to load the complete image. continuous code+data without skipping anything
		__bootdata_start = __code_end;
#endif
		__stat_data_start = .;
        *(.stacks.ish_defined_section.ulSystemStack*) //system stack must be stationary
        __stacks_segment_end = ALIGN(0x1000); //stacks region and stat_data region are overlapping
#if ISH_CONFIG_ENFORCE_SEGMENTS_LIMIT
        __ss_selector_low  = CREATE_GDT_SELECTOR_LOW(0,__stacks_segment_end,GDT_DATA_FLAGS);
        __ss_selector_high = CREATE_GDT_SELECTOR_HIGH(0,__stacks_segment_end,GDT_DATA_FLAGS);
#endif

        KEEP (*(.boot_data.ish_stationary_section*))
        __stat_data_end = .;
            
        __pinned_data_start = .;
        PINNED_OBJECTS_DATA
        OS_PINNED_OBJECTS_DATA
        *(.boot_data.ish_pinned_section*)
        __pinned_data_end = .;
        
        __pinned_bss_start = .;
        PINNED_OBJECTS_BSS
        OS_PINNED_OBJECTS_BSS
        *(.boot_data.bss.ish_pinned_section*)
        __pinned_bss_end = .;
        
	__init_data_start = .; 
        *(.data.ish_defined_section.init_flow*)
        . = ALIGN(0x1000);
        __init_data_end = .;
        __bootdata_end = ALIGN(0x1000);
    }

#if ISH_CONFIG_SUPPORT_CCM
	// in CCM memory, Data and code must reside in seperate banks
    . = ALIGN(ISH_CONFIG_SRAM_BANK_SIZE);
#if !ISH_CONFIG_SUPPORT_PAGING
    . = ( . + ISH_CONFIG_DATA_CCM_OFFSET);
#else
	//this is necessary to allign data section to bank size in GLV. this null page does not impact actually sram usage.
	. = ( . + ISH_BOOT_START); 
#endif
#endif

    .stacks_section : {
        __stacks_start = . ;
        *(.stacks.ish_defined_section.ucStacks*) //ucStacks should be page aligned, so there is no memory waste.
        . = ALIGN(0x1000);
        __stacks_end = . ;
    }
    	
    .code_section : {        
        __code_start = ALIGN(0x1000);
        *(.text*)          
        *(.text.unlikely.*)
        *(.rodata*)
		KEEP(*(.text.fw_test*))
		KEEP(*(.text.ULT*))
		KEEP(*(.text.VAL*))
        . = ALIGN(0x1000);
        __code_end = .;
#if ISH_CONFIG_ENFORCE_SEGMENTS_LIMIT
        __cs_selector_low  = CREATE_GDT_SELECTOR_LOW(0,__code_end,GDT_CODE_FLAGS);
        __cs_selector_high = CREATE_GDT_SELECTOR_HIGH(0,__code_end,GDT_CODE_FLAGS);
#endif
    }

    .data_section : {      
        . = ALIGN(0x1000);
        __data_start = .;         /* data_end is declared in the bss section below. */

        __internal_app_start = .;
        KEEP (*(.internal_app*) ) ;
        __internal_app_end = .;

        *(.data*)   
        __data_end = .;             
    }
    
    .bss : {    
        __bss_start = .; 
        *(.bss*) 
        *(COMMON*) 
        __bss_end = .; 
        __bss_segment_end = ALIGN(0x1000);
#if ISH_CONFIG_ENFORCE_SEGMENTS_LIMIT
        __ds_selector_low  = CREATE_GDT_SELECTOR_LOW(0,__bss_segment_end,GDT_DATA_FLAGS);
        __ds_selector_high = CREATE_GDT_SELECTOR_HIGH(0,__bss_segment_end,GDT_DATA_FLAGS);
#endif
    }
    
    __image_size                             = . - ISH_BOOT_START;
    MMU_VTOP_ADDR                            = ISH_CONFIG_SRAM_BASE - ISH_BOOT_START;
    pinned_bss_size_words                    = ((__pinned_bss_end - __pinned_bss_start) / 4);
	
#if ISH_CONFIG_SUPPORT_DEMAND_PAGING
    dataSegmentBasePhys                      = MMU_VTOP_ADDR;
#elif ISH_CONFIG_SUPPORT_PAGING
    dataSegmentBasePhys                      = MMU_VTOP_ADDR + ISH_CONFIG_DATA_CCM_OFFSET;
#else
	dataSegmentBasePhys     				= 0;
#endif

    __prepaging_ds_selector_low  = CREATE_GDT_SELECTOR_LOW(dataSegmentBasePhys,0xFFFFFFFF,GDT_DATA_FLAGS);
    __prepaging_ds_selector_high = CREATE_GDT_SELECTOR_HIGH(dataSegmentBasePhys,0xFFFFFFFF,GDT_DATA_FLAGS);

#if ISH_CONFIG_SUPPORT_PAGING
    _GdtPrePagingPhy = _GdtPrePaging + dataSegmentBasePhys;
    mmuPageDirectoryPhys = mmuPageDirectory   + dataSegmentBasePhys;
    mmuPageTablePhys     = mmuPageTable       + dataSegmentBasePhys;
    _TssPhy = _TssDes + dataSegmentBasePhys;
    _GdtEntriesP = _GdtEntries + dataSegmentBasePhys;
#else
    _GdtPrePagingPhy = _GdtPrePaging ;
    _TssPhy = _TssDes ;
    _GdtEntriesP = _GdtEntries ;
#endif

    __tss_selector_low  = CREATE_GDT_SELECTOR_LOW(_TssPhy,  0x67, 0x89);
    __tss_selector_high = CREATE_GDT_SELECTOR_HIGH(_TssPhy, 0x67, 0x89);
}

