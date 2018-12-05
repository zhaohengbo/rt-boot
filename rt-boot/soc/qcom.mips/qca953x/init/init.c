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
	
	qca953x_all_led_on();

}

void soc_deinit(void)
{
	//spiflash_reset(); //change flash mode back(For flash upper than 16MB)
	arch_deinit();
	qca953x_interrupt_deinit();
	qca953x_net_reset();
}