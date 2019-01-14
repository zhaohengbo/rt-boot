/*
 * Qualcomm/Atheros High-Speed UART driver
 *
 * Copyright (C) 2015 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2014 Mantas Pucka <mantas@8devices.com>
 * Copyright (C) 2008-2010 Atheros Communications Inc.
 *
 * Values for UART_SCALE and UART_STEP:
 * https://www.mail-archive.com/openwrt-devel@lists.openwrt.org/msg22371.html
 *
 * Partially based on:
 * Linux/drivers/tty/serial/qca953x_uart.c
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#include <kernel/rtthread.h>
#include <global/global.h>

#include <arch/timer.h>

void udelay(rt_uint32_t us)
{
	rt_uint32_t delay_cycle = us/5000;
	rt_uint32_t delay_remain = us%5000;

	//Should not passing value more than 10000
	//Otherwise,timer will overflow
	arch_timer_udelay(delay_remain);
	while(delay_cycle != 0)
	{
		arch_timer_udelay(5000);
		delay_cycle--;
	}
}

void mdelay(rt_uint32_t ms)
{
	udelay(ms*1000);
}

void msleep(rt_uint32_t ms)
{
	rt_thread_mdelay(ms);
}
