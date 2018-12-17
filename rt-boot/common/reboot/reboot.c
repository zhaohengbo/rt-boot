/*
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>

#include <board/reboot.h>

void system_reboot(void)
{
	rt_kprintf("\nRebooting...\n");
    board_reboot();
}
MSH_CMD_EXPORT(system_reboot, reboot system);