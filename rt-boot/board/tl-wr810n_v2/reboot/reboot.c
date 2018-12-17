/*
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <init/init.h>

#include <soc/qca953x/qca953x_reset.h>

void board_reboot(void)
{
	system_deinit();
	qca953x_full_reset();
}