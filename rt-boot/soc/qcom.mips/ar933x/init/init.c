/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <common/global.h>
#include <arch/init.h>
#include <soc/ar933x/ar933x_clock.h>
#include <soc/ar933x/ar933x_gpio.h>
#include <soc/ar933x/ar933x_hs_uart.h>

void soc_early_init(void)
{	
	arch_early_init();
}

void soc_late_init(void)
{	
	arch_late_init();
	ar933x_all_led_on();
	//ar933x_hsuart_init();
	//while(1)
		//ar933x_hsuart_putc('c');
}
