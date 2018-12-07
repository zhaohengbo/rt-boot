/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <global/global.h>
#include <main/main.h>
#include <arch/relocation.h>
#include <board/init.h>

static char version_info[64];

/* Here, we need to do early init */
void system_early_init(rt_uint32_t bootflag)
{
	rtboot_data.system_memsize = bootflag;
	board_early_init();
	arch_relocation();
}

void rtthread_startup(void)
{
    rt_hw_interrupt_disable();

    /* board level initialization
     * NOTE: please initialize heap inside board initialization.
     */
    rt_hw_board_init();
	
	rt_sprintf(version_info,"Version:1.0(%s)",__DATE__);
	rtboot_data.version_info = version_info;
	
	rt_kprintf("\nRT-Boot For Embedded Routers\n");
	rt_kprintf("Copyright (C) 2018 ZhaoXiaowei\n");
	rt_kprintf("%s\n",rtboot_data.version_info);
	
	rt_kprintf("Starting RTOS kernel...\n");

    /* timer system initialization */
    rt_system_timer_init();

    /* scheduler system initialization */
    rt_system_scheduler_init();

#ifdef RT_USING_SIGNALS
    /* signal system initialization */
    rt_system_signal_init();
#endif

    /* create init_thread */
    rt_application_init();

    /* timer thread initialization */
    rt_system_timer_thread_init();

    /* idle thread initialization */
    rt_thread_idle_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    while(1)
		;
}

/* Here, we are in relocated code */
void system_late_init(void)
{
	rt_hw_interrupt_disable();
	rtthread_startup();
	/* never reach here */
	while(1)
		;
}

void system_deinit(void)
{
	rt_hw_interrupt_disable();
	
	rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
	
	board_deinit();
}
