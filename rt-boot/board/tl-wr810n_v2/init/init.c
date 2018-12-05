/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <global/global.h>
#include <soc/init.h>
#include <board/console.h>
#include <board/network.h>
#include <soc/init.h>

void board_early_init(void)
{	
	soc_early_init();
}

void board_late_init(void)
{	
	soc_late_init();
	
	rtboot_data.system_baudrate = 115200;
	
	rt_hw_console_init();
	
	/* init hardware UART device */
	soc_console_init();
	
#ifdef RT_USING_CONSOLE
	/* set console device */
	rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
	
	board_network_init();
	
	rt_kprintf("\nRT-Boot For Embedded Routers\n");
	rt_kprintf("Copyright (C) 2018 ZhaoXiaowei\n");
	rt_kprintf("Version 1.0\n");
	rt_kprintf("Board: TEST_DEV_BOARD\n");
	rt_kprintf("Soc: AR7xxx/AR9xxx\n");
	rt_kprintf("Dram: %d MBytes\n",rtboot_data.system_memsize/1024/1024);
	rt_kprintf("Clocks: %d MHz\n",rtboot_data.system_frequency/1000/1000);
	
	//list_mips();
	
	rt_kprintf("Starting RTOS kernel...\n");
}

void rt_hw_board_init(void)
{
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#else
	board_late_init();
#endif
}

void board_deinit(void)
{
	soc_deinit();
}
