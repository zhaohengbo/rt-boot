/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <common/global.h>
#include <common/main.h>
#include <arch/relocation.h>
#include <board/init.h>

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

    /* show RT-Thread version */
    rt_show_version();

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
