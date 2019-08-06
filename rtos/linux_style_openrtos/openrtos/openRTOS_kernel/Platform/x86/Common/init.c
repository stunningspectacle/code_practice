/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2012 - 2014 Intel Corporation. All Rights Reserved.

   The source code contained or described herein and all documents related
   to the source code ("Material") are owned by Intel Corporation or its
   suppliers or licensors. Title to the Material remains with Intel Corporation
   or its suppliers and licensors. The Material contains trade secrets and
   proprietary and confidential information of Intel or its suppliers and
   licensors. The Material is protected by worldwide copyright and trade secret
   laws and treaty provisions. No part of the Material may be used, copied,
   reproduced, modified, published, uploaded, posted, transmitted, distributed,
   or disclosed in any way without Intel's prior express written permission.

   No license under any patent, copyright, trade secret or other intellectual
   property right is granted to or conferred upon you by disclosure or delivery
   of the Materials, either expressly, by implication, inducement, estoppel or
   otherwise. Any license under such intellectual property rights must be
   express and approved by Intel in writing.
--*/


#include "FreeRTOS.h"
#include "task.h"
#include "syslog.h"
#include "trace.h"
#include "ish_i2c.h"
#include "ish_sections.h"
#if ISH_CONFIG_SUPPORT_PAGING
#include <mmu.h>
#include <page_mgr.h>
#endif

#include <loader_common.h>
#include <sideband_gpio.h>
#include <sideband_gpio_defs.h>

#include <storage.h>
#include <dma.h>
#include <power_mgt_if.h>
#include <mctp.h>
#include <os_api.h> // LOCK_TYPE
#include <timer_high_res.h>
#include <hpet.h>
#include <app_manager.h>
#include <uart_dbg.h>

extern sram_page_db_entry_t * sramDataBase;
extern uint8_t sram_db_init_done;
extern uint32_t* sram_banks_powered;
extern debugger_fw_pg_t* dbg_pg_ptr;

#ifndef NULL
	#define NULL (void *)0
#endif

#if ISH_CONFIG_SUPPORT_ISH_HW
extern void pm_init();
#endif

LOCK_TYPE LOADER_MCTP_LOCK;

ldr_context ldr_ctx;

extern void ldr_init_mctp_client();
extern void ldr_get_snowball_data(OUT ish_loader_snowball_t * snowball);
extern void prvTaskExitError();

__pinned_kernel_code__ void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	(void)xTask;

	portDISABLE_INTERRUPTS();

   /* Look at pxCurrentTCB to see which task overflowed its stack. */
	printk("Stack Overflow, pxName = %s\n", pcTaskName);

	uint32_t ebp, return_addr = 0;
	int i = 0;

	asm volatile ("movl %%ebp, %0" : "=r" (ebp));

	while (i < 8) {
		return_addr = *(uint32_t *)(ebp + 4);
		if (return_addr == (uint32_t)prvTaskExitError)
			break;

		printk("%d) %#x\n", i, return_addr);

		ebp = *(uint32_t *)ebp;
		i++;
	}

	while (1);
}


__pinned_kernel_code__ void vApplicationMallocFailedHook( void )
{
	printk("Malloc Failed\n");
	while(1);
}


__pinned_kernel_code__ void vApplicationTickHook( void )
{
#if ISH_CONFIG_SUPPORT_PAGING
	extern void pageManagerTickHook();
	pageManagerTickHook();
#endif
}

__init_flow_code__  void updateSramDataBase(void)
{
    //update sramDataBase after mmu enabled, need make sure 4MB physcial 1:1 map enabled and no pagefault hapened before here. 
    struct aon_pointers * p_aon = (struct aon_pointers *)AON_TASK_BASE;
    sramDataBase        = (sram_page_db_entry_t*) p_aon->sram_db_ptr;
    sram_banks_powered  = &(p_aon->sram_banks_powered);
    sram_db_init_done = 1;

    //Update pointer to debugger struct and clear struct
    dbg_pg_ptr = p_aon->dbg_pg_ptr;
    memset(dbg_pg_ptr, 0x0, sizeof(debugger_fw_pg_t));
}

__init_flow_code__  void zeroBss(void)
{

extern unsigned int __bss_start;
extern unsigned int __bss_end;

    memset(&__bss_start, 0, (size_t)&__bss_end - (size_t)&__bss_start);

#if ISH_CONFIG_SUPPORT_DEMAND_PAGING
    memory_age(&__bss_start, (size_t)&__bss_end - (size_t)&__bss_start);
#endif
}

#ifndef DEMO
void init_services(void)
{
	ASSERT_CONST(sizeof(ish_manifest_ext_t) == ISH_EXT_MAN_SIZE);

#if ISH_CONFIG_SUPPORT_ISH_HW

	// TODO: remove comment after gdb is supported in OpenRTOS
    //gdb_agent_init();

#if (ISH_CONFIG_SUPPORT_TRACE_CONFIG) && \
	!(defined(VALIDATION) && defined(KERNEL_TEST) && defined(FPGA))
	// trace_init_full_config must be called before storage_init()
    // becasue on boot trace uses the storage's RX buffer and stops using it only after trace_init_full_config()

    trace_init_full_config();
#endif
#endif

#if !(defined(VALIDATION) && defined(KERNEL_TEST) && defined(FPGA)) // according to bsp config only CHV support IPC SEC
    //sotrage isn't supported in validation image since IPC SEC isn't opened
    storage_init();
#endif


#if !ISH_CONFIG_SUPPORT_ISH_HW


#if (ISH_CONFIG_SUPPORT_TRACE_CONFIG)
    // in Clanton, we must call trace_init after storage initialization and gdb_agent_init after trace initialization.
    clanton_trace_full_config();
#endif

    // TODO: remove comment after gdb is supported in Clanton and OpenRTOS
    //gdb_agent_init();
#endif
}
#endif // DEMO

static void tasks()
{
extern void pm_work_task();
extern void heci_rx_task();
extern void trace_worker();
extern void mctp_task();

    task_start("pm_work_task", 1600, 1, pm_work_task, NULL);
    task_start("heci_rx_task", 1600, 3, heci_rx_task, NULL);
    task_start("trace_worker", 1600, 8, trace_worker, NULL );
    task_start("mctp_task_param", 1600, 3, mctp_task, NULL);
}

__init_flow_code__ void main_init()
{

extern void heci_init(void);
extern void initHW(void);
extern void registerVecToHandler(void);
extern void console_init();
extern void init_printk(void);
extern void hh_init(void);

    ish_stat_reset();
    ish_stat_set_flag(lpsm_page_manager_initialize);

#if ISH_CONFIG_SUPPORT_HAMMOCKHARBOR
    /* init hh before any log output, NPK log needs it */
	hh_init();
#endif

	// enable basic trace in kernel
	init_printk();
	console_init();
	syslog(LOG_COMP_KERNEL|LOG_INFO, "ISS OpenRTOS based FW ver 0.1\n");

	// init HW
    registerVecToHandler();
    
#if ISH_CONFIG_SUPPORT_ISH_HW
    pm_init();
#endif    

    initHW();

    // we must init trace context (queue) before threads start
    trace_init_queue();
    i2c_syslog_init();
	uart_syslog_init();
	syslog(LOG_COMP_KERNEL|LOG_INFO, "moved to syslog over queue mode!\n");
	// init heci semaphores before threads start
	heci_init();

#ifndef DEMO

    LOADER_MCTP_LOCK = Create_LOCK(RES_LOADER_MCTP_LOCK);

	// setting an idle cooldown of 2500ms. avoid latency due to d0i3 exit/entry during load.

	pm_client_config_t pm_config = {0,0,LDR_TEMP_COOLDOWN,0};
    (void)pm_set_client_config(&pm_config);

    //Adding the timeout disable to kernel as well.
    //Once bringup is definitely integrated into BKC, this can be removed from kernel.
#if ISH_CONFIG_SUPPORT_ISH_HW && !defined(FPGA)
    dma_ocp_timeout_disable();
#endif

    uint32_t result;
    if( get_peripheral_config(PERIPHERAL_PMCTL_VALUE,&result,sizeof(result)) == SUCCESS)
    {
    	syslog(LOG_COMP_LOADER | LOG_INFO ,"PMCTL: 0x%x.\n", result);
    }


	ldr_get_snowball_data(&ldr_ctx.snowball);
	    syslog(LOG_COMP_LOADER | LOG_DEBUG, "dram_man_addr:%08x %08x\n",
	                            (unsigned int)(ldr_ctx.snowball.dram_manifest_addr >> 32),
	                            (unsigned int)(ldr_ctx.snowball.dram_manifest_addr & 0xFFFFFFFF));

#if ISH_CONFIG_SUPPORT_ISH_HW

	    // No MCTP in Validation kernel test app image
#if !(defined(VALIDATION) && defined(KERNEL_TEST) && defined(FPGA))
    //register to mctp
    ldr_init_mctp_client();
    //enable messages from host.
    syslog(LOG_COMP_LOADER | LOG_DEBUG ,"ready to receive host requests\n");
#endif
    //remove cooldown that was set previously. app will set it's own value later.
#if 0
    // Keep cooldown setting here, or fail to boot
    pm_config.cooldown_ms = 0;
    (void)pm_set_client_config(&pm_config);
#endif
#endif


	//Use hpet timer as the high res timer of the kernel
    struct timer_high_res_hw_interface hpet_timer_inf = {.init = hpet_init_secondary_timer,
                                                          .start = hpet_set_secondary_timer,
                                                          .stop = hpet_stop_secondary_timer,
                                                         };
    kernel_timer_high_res = timer_high_res_init(hpet_timer_inf);

#endif // DEMO

    tasks();

    init_services();

    load_internal_apps();

    task_abort(NULL);
}


void preSchedulerCode(void);
__init_flow_code__ int c_entry_point(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    create_rtos_trace_resources();

    RTOS_TRACE_ENABLE(TRACE_START);

#ifdef DEMO
    preSchedulerCode();
#endif

    task_start("main_init", 2048, 6, main_init, NULL);

    /* Initialize system and start the scheduler */
    vTaskStartScheduler();

    /* Will only reach here if there is insufficient heap available to start
       the scheduler. */
    for( ;; );
}



