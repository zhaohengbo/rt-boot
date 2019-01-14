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

static char board_name[32];

void board_early_init(void)
{	
	soc_early_init();
}

void board_late_init(void)
{	
	soc_late_init();
	
	rtboot_data.system_baudrate = 57600;
	
	rt_hw_console_init();
	
	/* init hardware UART device */
	soc_console_init();
	
	board_network_init();
	
#ifdef RT_USING_CONSOLE
	/* set console device */
	rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
	rt_sprintf(board_name,"MI 3C");
	rtboot_data.board_name = board_name;
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
