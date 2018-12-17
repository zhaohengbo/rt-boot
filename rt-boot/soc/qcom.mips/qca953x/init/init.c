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
#include <soc/qca953x/qca953x_clock.h>
#include <soc/qca953x/qca953x_ls_uart.h>
#include <soc/qca953x/qca953x_gpio.h>
#include <soc/qca953x/qca953x_irq.h>
#include <soc/qca953x/qca953x_reset.h>

static char soc_name[32];
static char clock_info[128];

void soc_early_init(void)
{	
	arch_early_init();
}

void soc_late_init(void)
{	
	arch_late_init();
	
	qca953x_interrupt_init();
	
	qca953x_clock_init();
	
	/* init operating system timer */	
    arch_timer_init();
	
	qca953x_gpio_init();

	rt_sprintf(soc_name,"QCA953X");
	rtboot_data.soc_name = soc_name;
	
    rt_sprintf(clock_info,"CPU:%dMhz,DDR:%dMhz,AHB:%dMhz,REF:%dMhz,SPI:%dMhz"
               ,rtboot_data.cpu_clk/1000/1000
               ,rtboot_data.ddr_clk/1000/1000
               ,rtboot_data.ahb_clk/1000/1000
               ,rtboot_data.ref_clk/1000/1000
               ,rtboot_data.spi_clk/1000/1000
               );
	rtboot_data.clock_info = clock_info;
}

void soc_deinit(void)
{
	//spiflash_reset(); //change flash mode back(For flash upper than 16MB)
	arch_deinit();
	qca953x_interrupt_deinit();
	qca953x_net_reset();
}
