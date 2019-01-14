/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <global/global.h>
#include <arch/init.h>
#include <arch/timer.h>
#include <arch/coprocessor.h>
#include <soc/mt7628/mt7628_clock.h>
#include <soc/mt7628/mt7628_lite_uart.h>
#include <soc/mt7628/mt7628_irq.h>

static char soc_name[32];
static char clock_info[128];

void soc_early_init(void)
{	
	arch_early_init();
}

void soc_late_init(void)
{	
	arch_late_init();
	
	mt7628_interrupt_init();
	
	mt7628_clock_init();
	
	/* init operating system timer */	
    arch_timer_init();
	
	//mt7628_gpio_init();

	rt_sprintf(soc_name,"MT7628");
	rtboot_data.soc_name = soc_name;
	
    rt_sprintf(clock_info,"CPU:%dMhz,BUS:%dMhz"
               ,rtboot_data.cpu_clk/1000/1000
               ,rtboot_data.bus_clk/1000/1000
               );
	rtboot_data.clock_info = clock_info;
}

void soc_deinit(void)
{
	//spiflash_reset(); //change flash mode back(For flash upper than 16MB)
	arch_deinit();
	mt7628_interrupt_deinit();
	//mt7628_net_reset();
}
